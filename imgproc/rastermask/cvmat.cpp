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
#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"

#include "./cvmat.hpp"

namespace imgproc {

namespace bitfield {

namespace detail {

template <typename Mask>
inline cv::Mat asCvMat(const Mask &mask, double pixelSize)
{
#if 0
    const auto size(mask.dims());
    cv::Mat m(pixelSize * size.height, pixelSize * size.width, CV_8UC1);

    for (int j(0), y(0); j < size.height; ++j, y += pixelSize) {
        for (int i(0), x(0); i < size.width; ++i, x += pixelSize) {
            for (int jj(0); jj < pixelSize; ++jj) {
                for (int ii(0); ii < pixelSize; ++ii) {
                    m.at<unsigned char>(y + jj, x + ii)
                        = (mask.get(i, j) ? 0xff : 0x00);
                }
            }
        }
    }

    return m;
#endif
    (void) mask;
    (void) pixelSize;
    return cv::Mat();
}

} // namespace detail

cv::Mat asCvMat(const RasterMask &mask, double pixelSize)
{
    return detail::asCvMat(mask, pixelSize);
}

} // namespace bitfield

namespace quadtree {

math::Size2 maskMatSize(const RasterMask &mask, double pixelSize)
{
    const auto size(mask.dims());
    return math::Size2(long(std::ceil(pixelSize * size.width))
                       , long(std::ceil(pixelSize * size.height)));
}

cv::Mat asCvMat(const RasterMask &mask, double pixelSize)
{
    auto ms(maskMatSize(mask, pixelSize));
    cv::Mat m(ms.height, ms.width, maskMatDataType(mask));
    return asCvMat(m, mask, pixelSize);
}

cv::Mat& asCvMat(cv::Mat &m, const RasterMask &mask, double pixelSize)
{
    // ensure m has proper size and type
    auto ms(maskMatSize(mask, pixelSize));
    m.create(ms.height, ms.width, maskMatDataType(mask));

    m = cv::Scalar(0);
    auto white(cv::Scalar(0xff));

    /* NB Tomas D. fixed calculation of end in a way it stopped working for 
     * pixelSize being whole number.
     * 
     * Fixme: fixed so that it works for whole numbers again while retaining 
     * Tomas D.'s functinality. However, this should be fixed by removing 
     * pixelSize entirely - possible scaling must be done outside because for 
     * rational pixelSize the rasterizing operation as implemented now is not 
     * well defined (there are overlaps of cells) 
     */ 
    
    uint fracAdj(std::round(pixelSize) - pixelSize != 0.0 ? 1 : 0);

    mask.forEachQuad([&](uint xstart, uint ystart, uint xsize
                         , uint ysize, bool)
    {
        cv::Point2i start(int(std::floor(pixelSize * xstart))
                          , int(std::floor(pixelSize * ystart)));
        cv::Point2i end(int(std::ceil(pixelSize * (xstart + xsize - fracAdj )))
                        , int(std::ceil(pixelSize * (ystart + ysize - fracAdj))));

        cv::rectangle(m, start, end, white, CV_FILLED, 4);
    }, RasterMask::Filter::white);

    return m;
}

} // namespace quadtree

} // namespace imgproc
