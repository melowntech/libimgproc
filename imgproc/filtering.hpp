/**
 * @file filtering.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image filering
 */

#ifndef IMGPROC_FILTERING_HPP
#define IMGPROC_FILTERING_HPP

#include <dbglog/dbglog.hpp>

#include <math/math.hpp>
#include <math/filters.hpp>
#include <boost/gil/gil_all.hpp>

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
        for (auto i(0); i < gil::num_channels<PixelType>::value; ++i) {
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

    double weightSum( 0.0 ), valueSum[10];

    for ( int i = 0; i < numChannels; i++ )
        valueSum[i] = 0.0;
    
    for ( int i = ll.y; i <= ur.y; i++ ) {
        for ( int j = ll.x; j <= ur.x; j++ ) {

            double weight = filter( j - pos.x, i - pos.y );

            if ( math::ccinterval( 0, (int) view.width() - 1, j ) &&
                 math::ccinterval( 0, (int) view.height() - 1, i ) ) {

                for ( int k = 0; k < numChannels; k++ )
                    valueSum[k] += weight * cpos(0,0)[k];

                /*LOG( debug ) << "[" << j << "," << i << "]: weight= "
                             << weight << ", value= "
                             << (int) cpos(0,0)[0];*/
            }
            else {

                for ( int k = 0; k < numChannels; k++ )
                    valueSum[k] += weight * (*defaultColor)[k];
            }

            weightSum += weight;
            cpos.x()++;
        }

        cpos += gil::point2<std::ptrdiff_t>( - ( ur.x - ll.x + 1 ), 1 );
    }

    typename SrcView::value_type retval;

    for ( int i = 0; i < numChannels; i++ ) {
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



} // namespace imgproc


#endif 

