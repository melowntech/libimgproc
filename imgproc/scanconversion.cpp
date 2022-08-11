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
#include "scanconversion.hpp"

namespace imgproc {

Scanline::Scanline(int y, float x1, float x2, float z1, float z2)
{
    this->y = y;

    if (x1 < x2) {
        this->x1 = x1, this->x2 = x2,
        this->z1 = z1, this->z2 = z2;
    }
    else {
        this->x1 = x2, this->x2 = x1,
        this->z1 = z2, this->z2 = z1;
    }
}


void scanConvertTriangle(const cv::Point3f pt[3], int ymin, int ymax,
                         std::vector<Scanline>& scanlines)
{
    int top = 0, mid = 1, bot = 2;

    if (pt[top].y > pt[mid].y) std::swap(top, mid);
    if (pt[mid].y > pt[bot].y) std::swap(mid, bot);
    if (pt[top].y > pt[mid].y) std::swap(top, mid);

    if (pt[bot].y <= ymin) return;
    if (pt[top].y >= ymax) return;

    /*
                top *
                   / \
              1  /    \
               /       \
             /          \ 2
       mid *_            \
              \ _         \
                  \ _      \
                 3    \ _   \
                          \ _* bot
    */

    float y = ceil(pt[top].y);
    if (y < ymin) y = ymin;
    if (y >= ymax) return;
    float yskip = y - pt[top].y;

    float iy1 = 1.0f / (pt[mid].y - pt[top].y);
    float iy2 = 1.0f / (pt[bot].y - pt[top].y);
    float iy3 = 1.0f / (pt[bot].y - pt[mid].y);

    float dx1 = (pt[mid].x - pt[top].x)*iy1;
    float dx2 = (pt[bot].x - pt[top].x)*iy2;
    float dx3 = (pt[bot].x - pt[mid].x)*iy3;

    float dz1 = (pt[mid].z - pt[top].z)*iy1;
    float dz2 = (pt[bot].z - pt[top].z)*iy2;
    float dz3 = (pt[bot].z - pt[mid].z)*iy3;

    float x1 = pt[top].x + dx1*yskip;
    float x2 = pt[top].x + dx2*yskip;

    float z1 = pt[top].z + dz1*yskip;
    float z2 = pt[top].z + dz2*yskip;

    while (y < pt[mid].y)
    {
        scanlines.emplace_back(static_cast<int>(y), x1, x2, z1, z2);

        y += 1.0f;
        if (y >= ymax) return;

        x1 += dx1;  z1 += dz1;
        x2 += dx2;  z2 += dz2;
    }

    x1 = pt[mid].x + dx3*(y - pt[mid].y);
    z1 = pt[mid].z + dz3*(y - pt[mid].y);

    while (y < pt[bot].y)
    {
        scanlines.emplace_back(y, x1, x2, z1, z2);

        y += 1.0f;
        if (y >= ymax) return;

        x1 += dx3;  z1 += dz3;
        x2 += dx2;  z2 += dz2;
    }
}

} // namespace imgproc
