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
 * @file scanconversion.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Triangle scan conversion
 */

#ifndef imgproc_scanconversion_hpp_included_
#define imgproc_scanconversion_hpp_included_

#include <vector>
#include <opencv2/core/core.hpp>

#include "math/geometry_core.hpp"
#include "math/geometry.hpp"

namespace imgproc {

//! Triangle scan conversion helper structure
struct Scanline
{
    int y;
    float x1, x2;
    float z1, z2;

    Scanline(int y, float x1, float x2, float z1, float z2);
};

//! Converts a triangle to a list of scanlines. The triangle vertices include
//! depth (Z), which is also interpolated.
void scanConvertTriangle(const cv::Point3f pt[3], int ymin, int ymax,
                         std::vector<Scanline>& scanlines);

//! Calls the specified operation for each pixel of a scanline
template<typename Operation>
void processScanline(const Scanline& sl, int xmin, int xmax, Operation op)
{
    float x = ceil(sl.x1);
    if (x < xmin) x = static_cast<float>(xmin);

    float dz = (sl.z2 - sl.z1) / (sl.x2 - sl.x1);
    float z = sl.z1 + dz*(x - sl.x1);

    while (x < sl.x2 && x < xmax) {
        op((int) x, sl.y, z);
        x += 1.0f;
        z += dz;
    }
}

//! Helper to call scanConvertTriangle with math::Point2
inline void scanConvertTriangle(
        const math::Point2 &a, const math::Point2 &b, const math::Point2 &c,
        int ymin, int ymax, std::vector<Scanline>& scanlines)
{
    cv::Point3f pt[3] = {
        { float(a(0)), float(a(1)), 0 },
        { float(b(0)), float(b(1)), 0 },
        { float(c(0)), float(c(1)), 0 },
    };
    scanConvertTriangle(pt, ymin, ymax, scanlines);
}

//! Helper to call scanConvertTriangle with math::Point3
inline void scanConvertTriangle(
        const math::Point3 &a, const math::Point3 &b, const math::Point3 &c,
        int ymin, int ymax, std::vector<Scanline>& scanlines)
{
    cv::Point3f pt[3] = {
        { float(a(0)), float(a(1)), float(a(2)) },
        { float(b(0)), float(b(1)), float(b(2)) },
        { float(c(0)), float(c(1)), float(c(2)) },
    };
    scanConvertTriangle(pt, ymin, ymax, scanlines);
}

template<typename Operation>
void dda(const math::Point2 & p1, const math::Point2 &p2, Operation op)
{
    math::Point2i begin(p1);
    math::Point2i end(p2);
    float p=math::length(p1-begin)/math::length(p2-p1);

    math::Point2 d(p2[0]-p1[0],p2[1]-p1[1]);

    double l = std::max(std::abs(d[0]),std::abs(d[1]));
    d = d*1/l;

    float dp = math::length(d)/math::length(p2-p1);

    int steps = std::max(std::abs(end[0]-begin[0]),std::abs(end[1]-begin[1]));

    math::Point2 cursor(begin);
    for(int i=0;i<steps+1; ++i){
        op(std::round(cursor[0]), std::round(cursor[1]), p);
        p+=dp;
        cursor+=d;
    }
}

template<typename Operation>
void dda(const cv::Point2f & p1, const cv::Point2f &p2, Operation op)
{
    dda(math::Point2( p1.x, p1.y ), math::Point2( p2.x, p2.y ),op);
}

} // namespace imgproc

#endif // imgproc_scanconversion_hpp_included_
