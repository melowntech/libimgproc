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
    this->forEachQuad([&](ushort x, ushort y, ushort xsize
                          , ushort ysize, bool white)
    {
        // rasterize quad
        ushort ex(x + xsize);
        ushort ey(y + ysize);

        for (ushort j(y); j < ey; ++j) {
            for (ushort i(x); i < ex; ++i) {
                op(i, j, white);
            }
        }
    }, filter);
}

template <typename Op>
inline void RasterMask::Node::descend(ushort x, ushort y, ushort size
                                      , const Op &op, Filter filter)
    const
{
    switch (type) {
    case GRAY: {
        // descend down
        ushort split = size / 2;
        ul->descend(x, y, split, op, filter);
        ll->descend(x, y + split, split, op, filter);
        ur->descend(x + split, y, split, op, filter);
        lr->descend(x + split, y + split, split, op, filter);
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
