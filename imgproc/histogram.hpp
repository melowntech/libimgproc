/**
 * @file histogram.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image histogram
 */

#ifndef IMGPROC_HISTOGRAM_HPP
#define IMGPROC_HISTOGRAM_HPP

namespace imgproc {

/* Obtain image histogram from a single channel view (gil based) */

template <typename View_t>
class Histogram {

public:
    Histogram_t( const View_t & view ) {

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


} // namespace imgproc

#endif // IMGPROC_HISTOGRAM_HPP

