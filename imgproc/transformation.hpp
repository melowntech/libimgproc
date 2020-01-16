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
 * @file transformation.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image transformation, based on a general 2D transformation concept
 */

#ifndef IMGPROC_TRANSFORMATION_HPP
#define IMGPROC_TRANSFORMATION_HPP

#include <type_traits>

#include "dbglog/dbglog.hpp"

#include "math/math_all.hpp"
#include "math/boost_gil_all.hpp"

#include "filtering.hpp"
#include "crop.hpp"

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
 * Scaling2 is a 2D scaling in pixel registration (pixel is area).
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

    template<typename T>
    math::Point2 map( const math::Point2_<T> & op ) const {

        return math::Point2( offX_ + scaleX_ * op(0), offY_ + scaleY_ * op(1) );
    }


    math::Point2 derivatives( const math::Point2i & op) const {
        (void) op;
        return math::Point2( scaleX_, scaleY_ );
    }


private:
    float scaleX_, scaleY_, offX_, offY_;
};

/**
 * GridScaling2 is a 2D scaling in grid registration (pixel is a point).
 * Models Mapping2 concept
 */
class GridScaling2 {
public:
    GridScaling2(const math::Size2 &srcSize, const math::Size2 &dstSize)
        : scaleX_(double(dstSize.width - 1) / (srcSize.width - 1))
        , scaleY_(double(dstSize.height - 1) / (srcSize.height - 1))
    {}

    template<typename T>
    math::Point2 map(const math::Point2_<T> &op) const {
        return math::Point2(scaleX_ * op(0), scaleY_ * op(1));
    }

    math::Point2 derivatives(const math::Point2i&) const {
        return math::Point2(scaleX_, scaleY_);
    }

private:
    double scaleX_, scaleY_;
};

/** Maps pixels from destination view to source crop
 */
class ReverseCroppingAndScaling2 {
public:
    template <typename T>
    ReverseCroppingAndScaling2(const Crop2_<T> &srcCrop
                               , const math::Size2 &dstSize)
        : scaleX_(float(srcCrop.width) / dstSize.width)
        , scaleY_(float(srcCrop.height) / dstSize.height)
        , offX_(scaleX_ * 0.5 - 0.5 + srcCrop.x)
        , offY_(scaleY_ * 0.5 - 0.5 + srcCrop.y)
    {}

    math::Point2 map(const math::Point2i &op) const {
        return { offX_ + scaleX_ * op(0), offY_ + scaleY_ * op(1) };
    }

    math::Point2 derivatives(const math::Point2i &op) const {
        (void) op;
        return { scaleX_, scaleY_ };
    }

private:
    float scaleX_;
    float scaleY_;
    float offX_;
    float offY_;
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
inline void scale(const SrcView & view1, const DstView & view2);

/** Same as above, use DefaultFilter.
 */
template <typename SrcView, typename DstView>
inline void scale(const SrcView &view1, const DstView &view2);

/** Crop area from src and scale it to fit to dstview.
 *
 *  \param LowPassFilter (template) filter used to reconstruct value in each
 *                       pixel
 *
 *  \param view1 source view
 *  \param view2 destination view
 *  \param srcCrop crop area from view1
 */
template <typename LowPassFilter2, typename SrcView, typename DstView
          , typename T>
inline void cropAndScale(const SrcView &view1, const DstView &view2
                         , const imgproc::Crop2_<T> &srcCrop);

/** Crop area from src and scale it to fit to dstview. Used default low pass
 * filter.
 *
 *  \param view1 source view
 *  \param view2 destination view
 *  \param srcCrop crop area from view1
 */
template <typename SrcView, typename DstView, typename T>
inline void cropAndScale(const SrcView &view1, const DstView &view2
                         , const imgproc::Crop2_<T> &srcCrop);

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
inline void scale(const SrcView & view1, const DstView & view2)
{
    Scaling2 scaling( math::Size2( view2.width(), view2.height() ),
              math::Size2( view1.width(), view1.height() ) );

    transform<LowPassFilter2>(scaling, view1, view2);
}

template <typename SrcView, typename DstView>
inline void scale(const SrcView &view1, const DstView &view2)
{
    return scale<DefaultFilter>(view1, view2);
}

template <typename LowPassFilter2, typename SrcView, typename DstView
          , typename T>
inline void cropAndScale(const SrcView & view1, const DstView &view2
                         , const imgproc::Crop2_<T> &srcCrop)
{
    // since transform traverses destination view and maps coordinates to src
    // viewport we must create reverse mapping

    ReverseCroppingAndScaling2
        op(srcCrop, math::Size2(view2.width(), view2.height()));
    transform<LowPassFilter2>(op, view1, view2);
}

template <typename SrcView, typename DstView, typename T>
inline void cropAndScale(const SrcView &view1, const DstView &view2
                         , const imgproc::Crop2_<T> &srcCrop)
{
    return cropAndScale<DefaultFilter>(view1, view2, srcCrop);
}

} // namespace imgproc


#endif
