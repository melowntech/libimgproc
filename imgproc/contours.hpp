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

#include <memory>
#include <vector>
#include <array>

#include "utility/enum-io.hpp"

#include "math/geometry_core.hpp"
#include "math/geometry.hpp"

#include "pixelorigin.hpp"
#include "rastermask/bitfield.hpp"

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

UTILITY_GENERATE_ENUM(ChainSimplification,
                      ((none))
                      ((simple))
                      ((rdp))
                      )

/** Contour finding algorithm parameters.
 */
struct ContourParameters {
    /** 0,0 is either at pixel center or at pixel corner
     */
    PixelOrigin pixelOrigin;

    /** Join adjacent straight segments into one segment.
     */
    ChainSimplification simplification;

    /** ChainSimplification::rdp algorithm maximum allowed segment error.
     */
    double rdpMaxError;

    ContourParameters()
        : pixelOrigin(PixelOrigin::center)
        , simplification(ChainSimplification::simple)
        , rdpMaxError(0.9)
    {}

    ContourParameters(PixelOrigin pixelOrigin)
        : pixelOrigin(pixelOrigin), simplification(ChainSimplification::simple)
    {}

    ContourParameters& setPixelOrigin(PixelOrigin pixelOrigin) {
        this->pixelOrigin = pixelOrigin; return *this;
    }

    ContourParameters& setSimplification(ChainSimplification simplification) {
        this->simplification = simplification; return *this;
    }

    ContourParameters& setRdpMaxError(double rdpMaxError) {
        this->rdpMaxError = rdpMaxError; return *this;
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

class FindContours {
public:
    FindContours(const math::Size2 &rasterSize, int colorCount
                 , const ContourParameters &params = ContourParameters());
    ~FindContours();

    /** Feed contour finder with value at given cell.
     */
    void operator()(int x, int y, int ul, int ur, int lr, int ll);

    const math::Size2 rasterSize() const;

    Contour::list contours();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/** Helper function for whole raster feed.
 */
template <typename ConstRaster>
Contour::list findContours(const ConstRaster &raster, int colorCount
                           , const ContourParameters &params
                           = ContourParameters());

// inlines

template <typename ConstRaster, typename Threshold>
Contour findContour(const ConstRaster &raster, const Threshold &threshold
                    , const ContourParameters &params)
{
    return findContour(bitfield::fromRaster(raster, threshold), params);
}

template <typename ConstRaster>
Contour::list findContours(const ConstRaster &raster, int colorCount
                           , const ContourParameters &params)
{
    const auto size(raster.size());
    auto xend(size.width - 1);
    auto yend(size.height - 1);

    FindContours fc(size, colorCount, params);

    // first row
    // first column
    fc(-1, -1, -1, colorCount
       , raster(0, 0)[0], colorCount);
    for (int i(0); i < xend; ++i) {
        fc(i, -1, colorCount, colorCount
           , raster(i + 1, 0)[0], raster(i, 0)[0]);
    }
    // last column
    fc(xend, -1, colorCount, -1
       , colorCount, raster(xend, 0)[0]);

    for (int j(0); j < yend; ++j) {
        // first column
        fc(-1, j, colorCount, raster(0, j)[0]
           , raster(0, j + 1)[0], colorCount);

        for (int i(0); i < xend; ++i) {
            fc(i, j, raster(i, j)[0], raster(i + 1, j)[0]
               , raster(i + 1, j + 1)[0], raster(i, j + 1)[0]);
        }

        // last column
        fc(xend, j, raster(xend, j)[0], colorCount
           , colorCount, raster(xend, j + 1)[0]);
    }

    // last row
    // first column
    fc(-1, yend, colorCount, raster(0, yend)[0]
       , colorCount, -1);
    for (int i(0); i < xend; ++i) {
        fc(i, yend, raster(i, yend)[0], raster(i + 1, yend)[0]
           , colorCount, colorCount);
    }
    // last column
    fc(xend, yend, raster(xend, yend)[0], colorCount
       , -1, colorCount);

    return fc.contours();
}

} // namespace imgproc

#endif // imgproc_contours_hpp_included_
