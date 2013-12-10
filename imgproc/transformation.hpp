/**
 * @file transformation.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image transformation, based on a general 2D transformation concept
 */

#ifndef IMGPROC_TRANSFORMATION_HPP
#define IMGPROC_TRANSFORMATION_HPP

#include <dbglog/dbglog.hpp>

#include <math/math_all.hpp>
#include <boost/gil/gil_all.hpp>

#include "filtering.hpp"

namespace imgproc {

/**
 * Mapping2 is a generic 2D->2D transformation function
 *
 * class Mapping2Concept {
 * public:
 *      math::Point2 map( const math::Point2i & op ) const;
 *      math::Point2 derivatives( const math::Point2i & op ) const;
 * };
 */

/**
 * Scaling2 is a 2D scaling in grid registration (pixel is area).
 * Models Mapping2 concept
 */

class Scaling2 {
    
public:
    Scaling2( const math::Size2 & srcSize, const math::Size2 & dstSize ) {

        scaleX_ = ( (float) dstSize.width ) / ( srcSize.width );
        scaleY_ = ( (float) dstSize.height ) / ( srcSize.height );
        offX_ = scaleX_ * 0.5 - 0.5;
        offY_ = scaleY_ * 0.5 - 0.5;
    };
    
    math::Point2 map( const math::Point2i & op ) const {

        return math::Point2( offX_ + scaleX_ * op(0), offY_ + scaleY_ * op(1) );
    }

    
    math::Point2 derivatives( const math::Point2i & op) const {
        (void) op;
        return math::Point2( scaleX_, scaleY_ );
    }

    
private:
    float scaleX_, scaleY_, offX_, offY_;
};


typedef math::SincHamming2 DefaultFilter;

/*
 * Transform between two views using a generic reverse mapping function
 */ 
template <typename LowPassFilter2, typename Mapping2
          , typename SrcView, typename DstView>
inline void transform(
        const Mapping2 & mapping,
        const SrcView & view1, const DstView & view2 );

/** Same as above, use DefaultFilter.
 */
template <typename Mapping2, typename SrcView, typename DstView>
inline void transform(
        const Mapping2 & mapping,
        const SrcView & view1, const DstView & view2 );

template <typename LowPassFilter2, typename SrcView, typename DstView>
inline void scale( const SrcView & view1, const DstView & view2 );

/** Same as above, use DefaultFilter.
 */
template <typename SrcView, typename DstView>
inline void scale(const SrcView &view1, const DstView &view2);

/* implementation */

template <typename LowPassFilter2, typename Mapping2
          , typename SrcView, typename DstView>
inline void transform(
        const Mapping2 & mapping,
        const SrcView & view1,
        const DstView & view2 ) {

    for ( int i = 0, ei(view2.height()); i < ei; i++ ) {

        typename DstView::x_iterator dstit = view2.row_begin( i );

        for ( int j = 0, ej(view2.width()); j < ej; j++ ) {

            math::Point2i dstpos( j, i );
            
            math::Point2 srcpos = mapping.map( dstpos );
            math::Point2 deriv = mapping.derivatives( dstpos );
            
            LowPassFilter2 filter(
                std::max( 2.0, 2.0 * deriv(0) ), std::max( 2.0, 2.0 * deriv(1) ) );

            *dstit++ = imgproc::reconstruct( view1, filter,
                 gil::point2<double>( srcpos(0), srcpos(1) ) );
        }
    }    
}

template <typename Mapping2, typename SrcView, typename DstView>
inline void transform(
        const Mapping2 & mapping,
        const SrcView & view1, const DstView & view2 )
{
    return transform<DefaultFilter>(mapping, view1, view2);
}

template <typename LowPassFilter2, typename SrcView, typename DstView>
inline void scale( const SrcView & view1, const DstView & view2 ) {
    Scaling2 scaling( math::Size2( view2.width(), view2.height() ),
              math::Size2( view1.width(), view1.height() ) );

    transform<LowPassFilter2>(scaling, view1, view2);
}

template <typename SrcView, typename DstView>
inline void scale(const SrcView &view1, const DstView &view2) {
    return scale<DefaultFilter>(view1, view2);
}

} // namespace imgproc


#endif
