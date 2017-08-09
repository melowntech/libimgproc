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
#include "math/geometry.hpp"

#include "./pixelorigin.hpp"
#include "./rastermask/bitfield.hpp"
#include "./detail/contours.hpp"

namespace imgproc {

/** Contour extracted from binary image.
 */
struct Contour {
    typedef std::vector<Contour> list;

    typedef bitfield::RasterMask Raster;
    typedef bitfield::RasterMask Border;

    typedef math::Polygon Ring;
    typedef math::MultiPolygon Rings;

    /** Found rings.
     */
    Rings rings;

    /** Border pixels.
     */
    Border border;

    // default ctor
    Contour(const math::Size2 &size = math::Size2(1, 1))
        : border(size, Border::InitMode::EMPTY)
    {}

    operator bool() const { return !rings.empty(); }
    bool operator!() const { return rings.empty(); }
};

/** Contour finding algorithm parameters.
 */
struct ContourParameters {
    /** 0,0 is either at pixel center or at pixel corner
     */
    PixelOrigin pixelOrigin;

    /** Join adjacent straight segments into one segment.
     */
    bool joinStraightSegments;

    ContourParameters()
        : pixelOrigin(PixelOrigin::center)
        , joinStraightSegments(true)
    {}

    ContourParameters(PixelOrigin pixelOrigin)
        : pixelOrigin(pixelOrigin), joinStraightSegments(true)
    {}

    ContourParameters& setPixelOrigin(PixelOrigin pixelOrigin) {
        this->pixelOrigin = pixelOrigin; return *this;
    }

    ContourParameters& setJoinStraightSegments(bool joinStraightSegments) {
        this->joinStraightSegments = joinStraightSegments; return *this;
    }
};

/** Find region contrours in const raster. Region is defined by pixels for wich
 *  threshold(x, y) return true.
 *
 * \param raster source raster
 * \param threshold thresholding function
 * \param params algorightm parameters
 * \return found contour
 */
template <typename ConstRaster, typename Threshold>
Contour findContour(const ConstRaster &raster, const Threshold &threshold
                    , const ContourParameters &params = ContourParameters());

/** Find region contrours in binary image represented by bitfield raster mask.
 *  Region is defined by pixels which are set in tha mask.
 *
 *  Contour orientation: if raster X grows to the right and raster Y grows
 *  downward then the extracted contour rings have this properties:
 *
 *      * contoured region is always to the right of the contour edges
 *      * outer rings have CW orientation
 *      * inner rings (holes) have CCW orientation
 *
 *  NB: If Y is flipped upside down (i.e. Y grows upward) then the properties
 *  are exactly oppsite: region is to the left of the edges, outer rings have
 *  CCW orientation and inner rings (holes) have CW orientation.
 *
 * \param raster source binary raster
 * \param params algorightm parameters
 * \return found contour
 */
Contour findContour(const Contour::Raster &raster
                    , const ContourParameters &params = ContourParameters());

/** Contour finder with internal state. Handles stable region boundaries for
 *  different regions inside common input.
 */
class FindContour : private detail::FindContourImpl {
public:
    FindContour(const ContourParameters &params = ContourParameters())
        : params_(params)
    {}

    template <typename ConstRaster, typename Threshold>
    Contour operator()(const ConstRaster &raster, const Threshold &threshold);

    Contour operator()(const Contour::Raster &raster);

private:
    struct Builder;
    friend class Builder;

    const ContourParameters params_;
};

/** Simplify contours.
 */
Contour::list simplify(Contour::list contours);

// inlines

template <typename ConstRaster, typename Threshold>
Contour FindContour::operator()(const ConstRaster &raster
                                , const Threshold &threshold)
{
    return operator()(bitfield::fromRaster(raster, threshold));
}

template <typename ConstRaster, typename Threshold>
Contour findContour(const ConstRaster &raster, const Threshold &threshold
                    , const ContourParameters &params)
{
    return FindContour(params)(raster, threshold);
}

inline Contour findContour(const Contour::Raster &raster
                           , const ContourParameters &params)
{
    return FindContour(params)(raster);
}

} // namespace imgproc

#endif // imgproc_contours_hpp_included_
