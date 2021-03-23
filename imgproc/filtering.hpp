/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @file filtering.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image filering
 */

#ifndef IMGPROC_FILTERING_HPP
#define IMGPROC_FILTERING_HPP

#if IMGPROC_HAS_OPENCV
#include <opencv2/core/core.hpp>
#endif

#include "math/math.hpp"
#include "math/geometry_core.hpp"
#include "math/filters.hpp"
#include "math/boost_gil_all.hpp"

#include "reconstruct.hpp"

namespace imgproc {

namespace gil = boost::gil;

/**
  * Filter a grayscale image (in x direction). GIL image views should be used.
  */
template <typename SrcView_t, typename DstView_t>
inline void rowFilter(
    const math::FIRFilter_t & filter,
    const SrcView_t & srcView,
    const DstView_t & dstView ) {

    for ( int y = 0; y < srcView.height(); y++ ) {
            typename SrcView_t::x_iterator srcxit = srcView.row_begin( y );
            typename DstView_t::x_iterator dstxit = dstView.row_begin( y );

            for ( int x = 0; x < srcView.width(); x++ ) {

                *dstxit = filter.gilConvolute( srcxit, x, srcView.width() );
                srcxit++; dstxit++;
            }

        }
    }


namespace detail {

template <typename PixelType>
class PixelLimits {
public:
    typedef typename gil::channel_type<PixelType>::type channel_type;

    PixelLimits()
        : min_(gil::channel_traits<channel_type>::min_value())
        , max_(gil::channel_traits<channel_type>::max_value())
    {
        auto numChannels = gil::num_channels<PixelType>::value;
        using ChannelT = decltype(numChannels);

        for (ChannelT i(0); i < numChannels; ++i) {
            zero_[i] = 0;
        }
    }

    double clamp(double value) const {
        if (value < min_) {
            return min_;
        } else if (value > max_) {
            return max_;
        }
        return value;
    }

    const PixelType& zero() const { return zero_; }

private:
    double min_;
    double max_;
    PixelType zero_;
};

template <>
class PixelLimits<gil::pixel<float, gil::gray_layout_t> > {
public:
    typedef gil::pixel<float, gil::gray_layout_t> pixel_type;
    PixelLimits() {
        zero_[0] = 0.f;
    }

    double clamp(double value) const {
        return value;
    }

    const pixel_type& zero() const { return zero_; }

private:
    pixel_type zero_;
};

} // namespace detail

/**
 * Reconstruct a value within an image with a continuous time filter.
 */

template <class SrcView, class Filter2>
inline typename SrcView::value_type reconstruct( const SrcView & view,
    const Filter2 & filter, const gil::point2<double> pos,
    boost::optional<typename SrcView::value_type> defaultColor = boost::none )
{
    detail::PixelLimits<typename SrcView::value_type> pl;
    const auto &zero(pl.zero());

    if (!defaultColor) { defaultColor = zero; }

    gil::point2<int> ll, ur;

    ll.x = (int) floor( pos.x - filter.halfwinx() );
    ll.y = (int) floor( pos.y - filter.halfwiny() );
    ur.x = (int) ceil( pos.x + filter.halfwinx() );
    ur.y = (int) ceil( pos.y + filter.halfwiny() );

    typename SrcView::xy_locator cpos = view.xy_at( ll.x, ll.y );

    auto numChannels = gil::num_channels<SrcView>::value;
    using ChannelT = decltype(numChannels);

    double weightSum( 0.0 ), valueSum[10];

    for ( ChannelT i = 0; i < numChannels; i++ )
        valueSum[i] = 0.0;
    
    for ( int i = ll.y; i <= ur.y; i++ ) {
        for ( int j = ll.x; j <= ur.x; j++ ) {

            double weight = filter( j - pos.x, i - pos.y );

            if ( math::ccinterval( 0, (int) view.width() - 1, j ) &&
                 math::ccinterval( 0, (int) view.height() - 1, i ) ) {

                for ( ChannelT k = 0; k < numChannels; k++ )
                    valueSum[k] += weight * cpos(0,0)[k];

                /*LOG( debug ) << "[" << j << "," << i << "]: weight= "
                             << weight << ", value= "
                             << (int) cpos(0,0)[0];*/
            }
            else {

                for ( ChannelT k = 0; k < numChannels; k++ )
                    valueSum[k] += weight * (*defaultColor)[k];
            }

            weightSum += weight;
            cpos.x()++;
        }

        cpos += gil::point2<std::ptrdiff_t>( - ( ur.x - ll.x + 1 ), 1 );
    }

    typename SrcView::value_type retval;

    for ( ChannelT i = 0; i < numChannels; i++ ) {
        if ( weightSum > 1E-15 ) {
            retval[i] = pl.clamp(valueSum[i] / weightSum);
        } else {
            retval[i] = zero[i];
        }
    }

    /*LOG( debug ) << pos.x << ", " << pos.y << ": ["
                 << ll.x << "," << ll.y << "]->[" << ur.x << "," << ur.y << "] : "
                 << (int) retval[0] << "(=" << valueSum[0] << "/" << weightSum << ")";*/
        
    return retval;
}


/**
 * Reconstruct a value from cv::Mat with a continuous time filter.
 * Note: only available when OpenCV is configured.
 */

#if IMGPROC_HAS_OPENCV
template<typename MatType, typename Filter2>
MatType reconstruct(const cv::Mat &mat, const Filter2 &filter,
                    const math::Point2 &pos, MatType defaultColor = MatType())
{
    int x1(floor(pos(0) - filter.halfwinx()));
    int x2(ceil (pos(0) + filter.halfwinx()));
    int y1(floor(pos(1) - filter.halfwiny()));
    int y2(ceil (pos(1) + filter.halfwiny()));

    int numChannels = mat.channels();

    double weightSum(0), valueSum[10];
    for (int i = 0; i < numChannels; i++)
        valueSum[i] = 0.0;

    for (int i = y1; i <= y2; i++)
    for (int j = x1; j <= x2; j++)
    {
        double weight(filter(j - pos(0), i - pos(1)));

        if (i >= 0 && i < mat.rows && j >= 0 && j < mat.cols)
        {
            const MatType &value(mat.at<MatType>(i, j));
            for (int k = 0; k < numChannels; k++)
                valueSum[k] += weight * value[k];
        }
        else {
            for (int k = 0; k < numChannels; k++)
                valueSum[k] += weight * defaultColor[k];
        }

        weightSum += weight;
    }

    MatType retval;
    for (int i = 0; i < numChannels; i++) {
        if (weightSum > 1e-15) {
            retval[i] = cv::saturate_cast<typename MatType::value_type>
                                (valueSum[i] / weightSum);
        } else {
            retval[i] = 0;
        }
    }

    return retval;
}
#endif

} // namespace imgproc

#endif
