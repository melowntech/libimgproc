/**
 * @file rastermask/quadtree.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (quadtree): inline functions.
 */

#ifndef imgproc_rastermask_inline_quadtree_hpp_included_
#define imgproc_rastermask_inline_quadtree_hpp_included_

namespace imgproc { namespace quadtree {

template <typename Op>
inline void RasterMask::forEachQuad(const Op &op, Filter filter) const
{
    root_.descend(0, 0, quadSize_, op, filter);
}

template <typename Op>
inline void RasterMask::forEach(const Op &op, Filter filter) const
{
    this->forEachQuad([&](uint x, uint y, uint xsize, uint ysize, bool white)
    {
        // rasterize quad
        uint ex(x + xsize);
        uint ey(y + ysize);

        for (uint j(y); j < ey; ++j) {
            for (uint i(x); i < ex; ++i) {
                op(i, j, white);
            }
        }
    }, filter);
}

template <typename Op>
inline void RasterMask::Node::descend(uint x, uint y, uint size
                                      , const Op &op, Filter filter)
    const
{
    switch (type) {
    case GRAY: {
        // descend down
        uint split = size / 2;
        children->ul.descend(x, y, split, op, filter);
        children->ll.descend(x, y + split, split, op, filter);
        children->ur.descend(x + split, y, split, op, filter);
        children->lr.descend(x + split, y + split, split, op, filter);
        return;
    }

    case BLACK:
        // filter out white quads
        if (filter == Filter::white) { return; }
        break;

    case WHITE:
        // filter out black quads
        if (filter == Filter::black) { return; }
        break;
    }

    // call operation for black/white node
    op(x, y, ((x + size) > mask.sizeX_) ? (mask.sizeX_ - x) : size
       , ((y + size) > mask.sizeY_) ? (mask.sizeY_ - y) : size
       , (type == WHITE));
}

} } // namespace imgproc::quadtree

#endif // imgproc_rastermask_inline_quadtree_hpp_included_
