/**
 * @file bitfield.cpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#include <iostream>
#include <stdexcept>
#include <numeric>
#include <cmath>

#include "dbglog/dbglog.hpp"
#include "math/math.hpp"

#include "./bitfield.hpp"

namespace imgproc { namespace bitfield {

namespace {
math::Extents2i extents(const boost::optional<imgproc::Crop2> &refRoi
                        , const math::Size2 &size, double sx, double sy)
{
    if (!refRoi) {
        // full
        return { 0, 0
                , int(std::ceil(size.width / sx))
                , int(std::ceil(size.height / sy)) };
    }

    math::Extents2i e(refRoi->x, refRoi->y, (refRoi->x + refRoi->width)
                      , (refRoi->y + refRoi->height));

    // limit to size
    if (e.ll(0) < 0) { e.ll(0) = 0; }
    if (e.ll(1) < 0) { e.ll(1) = 0; }
    if (e.ur(0) > size.width) { e.ur(0) = size.width; }
    if (e.ur(1) > size.height) { e.ur(1) = size.height; }

    // scale to mathc mask size
    return { int(std::floor(e.ll(0) / sx)), int(std::floor(e.ll(1) / sy))
            , int(std::ceil(e.ur(0) / sx)), int(std::ceil(e.ur(1) / sy)) };
}

} // namespace

int radius(const RasterMask &m
           , const boost::optional<math::Size2> &refSize
           , const boost::optional<imgproc::Crop2> &refRoi)
{
    auto size(m.dims());
    auto rs(refSize ? *refSize : size);

    double sx(double(rs.width) / size.width);
    double sy(double(rs.height) / size.height);

    // calculate roi extents
    auto roi(extents(refRoi, rs, sx, sy));

    // find out center
    double cx(math::size(roi).width / 2.);
    double cy(math::size(roi).height / 2.);

    // radius^2
    double r2(0);

    for (int j(roi.ll(1)); j < roi.ur(1); ++j) {
        for (int i(roi.ll(0)); i < roi.ur(0); ++i) {
            if (m.get(i, j)) {
                // white pixel -> calculate radius^2
                auto nr2(math::sqr((i - cx) * sx)
                         + math::sqr((j - cy) * sy));
                if (nr2 > r2) {
                    // larger value
                    r2 = nr2;
                }
            }
        }
    }

    // return radius (as integer)
    return std::sqrt(r2);
}

} } // namespace imgproc::quadtree
