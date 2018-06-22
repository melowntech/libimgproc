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
 * Triangle rasterizer.
 */

#ifndef imgproc_rasterizer_hpp_included_
#define imgproc_rasterizer_hpp_included_

#include "math/geometry_core.hpp"

#include "./scanconversion.hpp"

namespace imgproc {

class Rasterizer {
public:
    Rasterizer(const math::Extents2i &extents)
        : extents_(extents)
    {}

    Rasterizer(const math::Size2 &size)
        : extents_(0, 0, size.width, size.height)
    {}

    Rasterizer(int width, int height)
        : extents_(0, 0, width, height)
    {}

    template <typename T, typename Operation>
    void operator()(const math::Point2_<T> &a, const math::Point2_<T> &b
                    , const math::Point2_<T> &c, const Operation &op)
    {
        cv::Point3f pt[3] = {
            { float(a(0)), float(a(1)), 0.f },
            { float(b(0)), float(b(1)), 0.f },
            { float(c(0)), float(c(1)), 0.f }
        };

        run(pt, op);
    }

    template <typename T, typename Operation>
    void operator()(const math::Point3_<T> &a, const math::Point3_<T> &b
                    , const math::Point3_<T> &c, const Operation &op)
    {
        cv::Point3f pt[3] = {
            { float(a(0)), float(a(1)), float(a(2)) },
            { float(b(0)), float(b(1)), float(b(2)) },
            { float(c(0)), float(c(1)), float(c(2)) }
        };

        run(pt, op);
    }

    template <typename Operation>
    void operator()(float a1, float a2, float a3
                    , float b1, float b2, float b3
                    , float c1, float c2, float c3
                    , const Operation &op)
    {
        cv::Point3f pt[3] = {
            { a1, a2, a3 },
            { b1, b2, b3 },
            { c1, c2, c3 },
        };
        run(pt, op);
    }

private:
    template <typename Operation>
    void run(const cv::Point3f pt[3], const Operation &op)
    {
        scanlines_.clear();
        scanConvertTriangle(pt, extents_.ll(1), extents_.ur(1), scanlines_);
        for (const auto &sl : scanlines_) {
            processScanline(sl, extents_.ll(0), extents_.ur(0), op);
        }
    }

    const math::Extents2i extents_;
    std::vector<Scanline> scanlines_;
};


} // namespace imgproc

#endif // imgproc_rasterizer_hpp_included_
