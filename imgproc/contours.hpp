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

#ifndef imgproc_contours_hpp_included_
#define imgproc_contours_hpp_included_

#include <vector>

#include "math/geometry_core.hpp"

#include "./rastermask/bitfield.hpp"
#include "./detail/contours.hpp"

namespace imgproc {

typedef std::vector<math::Points2> Contours;

/** Find region contrours in const raster. Region is defined by pixels for wich
 *  threshold(x, y) return true.
 */
template <typename ConstRaster, typename Threshold>
Contours findContours(const ConstRaster &raster, const Threshold &threshold);

/** Find region contrours in binary image represented by bitfield raster mask.
 *  Region is defined by pixels which are set in tha mask.
 */
Contours findContours(const bitfield::RasterMask &mask);

/** Contour finder with internal state. Handles stable region boundaries for
 *  different regions inside common input.
 */
class FindContours : private detail::FindContoursImpl {
public:
    template <typename ConstRaster, typename Threshold>
    Contours operator()(const ConstRaster &raster, const Threshold &threshold);

    Contours operator()(const bitfield::RasterMask &mask);

    struct Builder;
    friend class Builder;
};

// inlines

template <typename ConstRaster, typename Threshold>
Contours FindContours::operator()(const ConstRaster &raster
                                   , const Threshold &threshold)
{
    return operator()(bitfield::fromRaster(raster, threshold));
}

template <typename ConstRaster, typename Threshold>
Contours findContours(const ConstRaster &raster, const Threshold &threshold)
{
    return FindContours()(raster, threshold);
}

inline Contours findContours(const bitfield::RasterMask &mask)
{
    return FindContours()(mask);
}

} // namespace imgproc

#endif // imgproc_contours_hpp_included_
