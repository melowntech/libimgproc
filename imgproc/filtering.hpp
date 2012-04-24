/**
 * @file filtering.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image filering
 */

#ifndef IMGPROC_FILTERING_HPP
#define IMGPROC_FILTERING_HPP

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

template <class SrcView>
typename SrcView::value_type reconstruct( const SrcView & view,
    const math::Filter2_t & filter, const gil::point2<double>  pos ) {

    gil::point2<int> ll, ur;

    /*ll.x = std::max( 0, (int) floor( pos.x - filter.halfwindowX() ) );
    ll.y = std::max( 0, (int) floor( pos.y - filter.halfwindowY() ) );
    ur.x = std::min( (int) view.width() - 1,
                     (int) ceil( pos.x + filter.halfwindowX() ) );
    ur.y = std::min( (int) view.height() - 1,
                     (int) ceil( pos.y + filter.halfwindowY() ) );*/
    ll.x = (int) floor( pos.x - filter.halfwindowX() );
    ll.y = (int) floor( pos.y - filter.halfwindowY() );
    ur.x = (int) ceil( pos.x + filter.halfwindowX() );
    ur.y = (int) ceil( pos.y + filter.halfwindowY() );

    typename SrcView::xy_locator cpos = view.xy_at( ll.x, ll.y );

    double weightSum( 0.0 ), valueSum[10];

    for ( int i = 0; i < gil::num_channels<SrcView>::value; i++ )
        valueSum[i] = 0.0;
    
    
    for ( int i = ll.y; i <= ur.y; i++ ) {
        for ( int j = ll.x; j <= ur.x; j++ ) {

            double weight = filter( j - pos.x, i - pos.y );

            for ( int k = 0; k < gil::num_channels<SrcView>::value; k++ ) {

                if ( math::ccinterval( 0, (int) view.width() - 1, j ) &&
                     math::ccinterval( 0, (int) view.height() - 1, i ) )
                    valueSum[k] += weight * cpos(0,0)[k];
            }

            weightSum += weight;
            cpos.x()++;
        }

        cpos += gil::point2<std::ptrdiff_t>( - ( ur.x - ll.x + 1 ), 1 );
    }

    typename SrcView::value_type retval;

    for ( int i = 0; i < gil::num_channels<SrcView>::value; i++ )
        if ( weightSum > 1E-15 )
            retval[i] = valueSum[i] / weightSum;
        else
            retval[i] = 0;

    return retval;
}



} // namespace imgproc






#endif 

