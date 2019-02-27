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
#include "dbglog/dbglog.hpp"

#include "transform.hpp"
#include "cvmat.hpp"

namespace imgproc { namespace quadtree {

namespace {

inline math::Point2 trans(const Matrix2x3 &tr, int x, int y)
{
    return {
        tr(0, 0) * x + tr(0, 1) * y + tr(0, 2)
        , tr(1, 0) * x + tr(1, 1) * y + tr(1, 2)
     };
}

} // namespace

RasterMask transform(const RasterMask &mask, const math::Size2 &size
                     , const Matrix2x3 &trafo)
{
    // kernel sizes (from scaling factor)
    double kw(trafo(0, 0) / 2.0);
    double kh(trafo(1, 1) / 2.0);

    auto m(asCvMat(mask));

    auto clamp([](int i, int max) -> int
    {
        if (i < 0) { return 0; }
        if (i > max) { return max; }
        return i;
    });

    // both ends of kernel are inclusive -> clip to -1 and use <=
    const int xMax(m.cols - 1);
    const int yMax(m.rows - 1);
    auto scan([&](const math::Point2 &p) -> bool
    {
        for (int y(clamp(std::floor(p(1) - kh), yMax))
                 , ey(clamp(std::ceil(p(1) + kh), yMax));
             y <= ey; ++y)
        {
            for (int x(clamp(std::floor(p(0) - kw), xMax))
                     , ex(clamp(std::ceil(p(0) + kw), xMax));
                 x <= ex; ++x)
            {
                if (!m.at<std::uint8_t>(y, x)) {
                    return false;
                }
            }
        }
        return true;
    });

    RasterMask out(size, RasterMask::EMPTY);
    for (int j(0); j < size.height; ++j) {
        for (int i(0); i < size.width; ++i) {
            if (scan(trans(trafo, i, j))) {
                out.set(i, j);
            }
        }
    }

    // done
    return out;
}

} } // namespace imgproc::quadtree
