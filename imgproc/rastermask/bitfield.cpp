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

int radius(const RasterMask &m
           , const boost::optional<math::Size2> &refSize
           , const boost::optional<imgproc::Crop2> &refRoi)
{
    (void) refRoi;

    auto size(m.dims());
    auto rs(refSize ? *refSize : size);

    double sx(double(rs.width) / size.width);
    double sy(double(rs.height) / size.height);

    // find out center
    double cx(rs.width / 2.);
    double cy(rs.height / 2.);

    // radius^2
    double r2(0);

    for (int j(0); j < size.height; ++j) {
        for (int i(0); i < size.width; ++i) {
            if (m.get(i, j)) {
                // white pixel -> calculate radius^2
                auto nr2(math::sqr((i * sx) - cx) + math::sqr((j * sy) - cy));
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
