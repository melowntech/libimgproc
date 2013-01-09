/**
 * @file histogram.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image histogram
 */

#ifndef IMGPROC_HISTOGRAM_HPP
#define IMGPROC_HISTOGRAM_HPP

#include <cmath>
#include <limits>

#include <boost/gil/gil_all.hpp>

namespace imgproc {


namespace gil = boost::gil;

namespace detail {
    template<typename T> struct numeric_limits;

    template<> struct numeric_limits<signed char> {
        static const signed char max = CHAR_MAX;
    };

    template<> struct numeric_limits<unsigned char> {
        static const unsigned char max = UCHAR_MAX;
    };

    template<> struct numeric_limits<short> {
        static const short max = SHRT_MAX;
    };

    template<> struct numeric_limits<unsigned short> {
        static const unsigned short max = USHRT_MAX;
    };
} // namespace detail

/* Obtain image histogram from a single channel view (gil based) */

template <typename View>
class Histogram {
public:
    typedef typename gil::channel_type<View>::type channel_type;
    static const channel_type max = detail::numeric_limits<channel_type>::max;

    Histogram(const View &view, channel_type lowerBound = 0
              , channel_type upperBound = max)
        : values{0}, total(0)
    {
        // TODO: work with Y channel, using Y or G so far
        const int channel((view.num_channels() == 3) ? 1 : 0);
        for (const auto &pixel : view) {
            auto value(pixel[channel]);
            if ((value >= lowerBound) && (value <= upperBound)) {
                values[value]++;
                ++total;
            }
        }
    }

    /**
     * Return the least threshold value such that given share of pixels is less
     * or equal to it
     */
    channel_type threshold( const float ratio ) const {
        const float thresholdCount = ratio * total;
        uint count = 0;
        for ( uint i = 0; i <= std::numeric_limits<channel_type>::max(); i++ )
        {
            count += values[ i ];
            if ( count >= thresholdCount ) return i;
        }
        return max;
    }

private:
    uint values[max + 1ul];
    uint total;
};

template <typename View>
Histogram<View>
histogram(const View &v
          , typename Histogram<View>::channel_type lowerBound = 0
          , typename Histogram<View>::channel_type upperBound
          = Histogram<View>::max)
{
    return Histogram<View>(v, lowerBound, upperBound);
}

template <typename SrcView>
void stretchValues(const SrcView &src
                   , const typename gil::channel_type<SrcView>::type &lb
                   , const typename gil::channel_type<SrcView>::type &ub)
{
    typedef typename gil::channel_type<SrcView>::type channel_type;

    const float max(std::numeric_limits<channel_type>::max());
    const float fmax(max);

    // TODO: work with YUV
    for ( int i = 0; i < src.height(); i++ ) {

        typename SrcView::x_iterator sit = src.row_begin( i );

        for ( int j = 0; j < src.width(); j++ ) {

            for ( int k = 0; k < gil::num_channels<SrcView>::value; k++ ) {

                if ( (*sit)[k] < lb ) {
                    (*sit)[k] = 0;
                } else if ( (*sit)[k] > ub ) {
                    (*sit)[k] = max;
                } else {
                    (*sit)[k] = channel_type((fmax * ((*sit)[k] - lb))
                                             / ( ub - lb ));
                }
            }

            ++sit;
        }
    }
}


} // namespace imgproc

#endif // IMGPROC_HISTOGRAM_HPP

