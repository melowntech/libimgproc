/**
 * @file filtering.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image filering
 */

#ifndef IMGPROC_FILTERING_HPP
#define IMGPROC_FILTERING_HPP

#include <dbglog/dbglog.hpp>

#include <math/filters.hpp>
#include <boost/gil/gil_all.hpp>

namespace imgproc {


namespace gil = boost::gil;

/**
  * Filter a grayscale image (in x direction). GIL image views should be used.
  */
template <typename SrcView_t, typename DstView_t>
void rowFilter(
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


/**
 * Reconstruct a value within an image with a continuous time filter.
 */

template <class SrcView, class Filter2>
typename SrcView::value_type reconstruct( const SrcView & view,
    const Filter2 & filter, const gil::point2<double>  pos ) {

    gil::point2<int> ll, ur;

    ll.x = (int) floor( pos.x - filter.halfwinx() );
    ll.y = (int) floor( pos.y - filter.halfwiny() );
    ur.x = (int) ceil( pos.x + filter.halfwinx() );
    ur.y = (int) ceil( pos.y + filter.halfwiny() );

    typename SrcView::xy_locator cpos = view.xy_at( ll.x, ll.y );

    double weightSum( 0.0 ), valueSum[10];

    for ( int i = 0; i < gil::num_channels<SrcView>::value; i++ )
        valueSum[i] = 0.0;
    
    for ( int i = ll.y; i <= ur.y; i++ ) {
        for ( int j = ll.x; j <= ur.x; j++ ) {

            double weight = filter( j - pos.x, i - pos.y );

            if ( math::ccinterval( 0, (int) view.width() - 1, j ) &&
                 math::ccinterval( 0, (int) view.height() - 1, i ) ) {

                for ( int k = 0; k < gil::num_channels<SrcView>::value; k++ )
                    valueSum[k] += weight * cpos(0,0)[k];

                /*LOG( debug ) << "[" << j << "," << i << "]: weight= "
                             << weight << ", value= "
                             << (int) cpos(0,0)[0];*/

            }

            weightSum += weight;
            cpos.x()++;
        }

        cpos += gil::point2<std::ptrdiff_t>( - ( ur.x - ll.x + 1 ), 1 );
    }

    typename SrcView::value_type black, white;
    gil::color_convert( gil::gray8_pixel_t( 0 ), black );
    gil::color_convert( gil::gray8_pixel_t( 0xff ), white );
    
    typename SrcView::value_type retval;

    for ( int i = 0; i < gil::num_channels<SrcView>::value; i++ )
        if ( weightSum > 1E-15 )
            retval[i] = std::min( std::max(
                            valueSum[i] / weightSum,
                            (double) black[i] ), (double) white[i] );
        else
            retval[i] = black[i];

    /*LOG( debug ) << pos.x << ", " << pos.y << ": ["
                 << ll.x << "," << ll.y << "]->[" << ur.x << "," << ur.y << "] : "
                 << (int) retval[0] << "(=" << valueSum[0] << "/" << weightSum << ")";*/
        
    return retval;
}



} // namespace imgproc


#endif 

