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
void rowFilter( const math::FIRFilter_t & filter, const SrcView_t & srcView, const DstView_t & dstView ) {

        for ( int y = 0; y < srcView.height(); y++ ) {
            typename SrcView_t::x_iterator srcxit = srcView.row_begin( y );
            typename DstView_t::x_iterator dstxit = dstView.row_begin( y );

            for ( int x = 0; x < srcView.width(); x++ ) {

                *dstxit = filter.gilConvolute( srcxit, x, srcView.width() );
                srcxit++; dstxit++;
            }

        }
    }


} // namespace imgproc

#endif 

