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

#ifndef imgproc_contours_svg_hpp_included_
#define imgproc_contours_svg_hpp_included_

#include <memory>
#include <vector>
#include <array>

#include "./contours.hpp"
#include "./svg.hpp"

namespace imgproc { namespace svg {

template <typename Color>
void draw(std::ostream &os, const Contour &contour, const Color &color)
{
    for (const auto &ring : contour.rings) {
        if (ring.empty()) { continue; }

        os << "<polygon points=\"";
        for (const auto &point : ring) {
            os << point(0) << ',' << point(1) << ' ';
        }

        os << "\" style=\"fill:none;"
           << imgproc::svg::stroke(color, 0.8)
           << ";stroke-width:1\""
           << " vector-effect=\"non-scaling-stroke\""
           << " />\n";
    }
}

} } // namespace imgproc::svg

#endif // imgproc_contours_svg_hpp_included_
