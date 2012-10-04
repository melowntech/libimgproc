/**
 * @file histogram.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image histogram
 */

#ifndef IMGPROC_HISTOGRAM_HPP
#define IMGPROC_HISTOGRAM_HPP

#include <boost/gil/gil_all.hpp>

namespace imgproc {


namespace gil = boost::gil;

/* Obtain image histogram from a single channel view (gil based) */

template <typename View_t>
class Histogram {

public:
    Histogram( const View_t & view ) {

        std::fill( values, values + 256, 0 ); total = 0;
        for ( typename View_t::iterator it = view.begin();
             it < view.end(); ++it ) {

            values[ (unsigned char) round( (*it)[0] ) ]++; total++;
             }
    }

    /**
     * Return the least threshold value such that given share  of
     * pixels is less or equal to it
     */
    unsigned char threshold( const float ratio ) const {

        const float thresholdCount = ratio * total;
        uint count = 0;
        for ( uint i = 0; i < 256; i++ ) {
            count += values[ i ];
            if ( count >= thresholdCount ) return i;
        }
        return 255;
    }

private:
    uint values[ 256 ];
    uint total;
};

template <typename View>
Histogram<View> histogram(const View &v)
{
    return Histogram<View>(v);
}

template <typename SrcView>
inline void stretchValues( SrcView & src,
               const typename gil::channel_type<SrcView>::type & lb,
               const typename gil::channel_type<SrcView>::type & ub ) {

    typedef typename gil::channel_type<SrcView>::type Result;
    
    for ( int i = 0; i < src.height(); i++ ) {

        typename SrcView::x_iterator sit = src.row_begin( i );
        
        for ( int j = 0; j < src.width(); j++ ) {
            
            for ( int k = 0; k < gil::num_channels<SrcView>::value; k++ ) {

                if ( (*sit)[k] < lb ) { (*sit)[k] = 0; continue; }
                if ( (*sit)[k] > ub ) { (*sit)[k] = 255; continue; }
                
                (*sit)[k] = (Result) 255
                    * ( (float) (*sit)[k] - lb ) / ( ub -lb );
            }

            sit++;
        }
    }
}


} // namespace imgproc

#endif // IMGPROC_HISTOGRAM_HPP

