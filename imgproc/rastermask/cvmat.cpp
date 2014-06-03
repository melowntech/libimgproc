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

cv::Mat asCvMat(const RasterMask &mask, double pixelSize)
{
    const auto size(mask.dims());
    cv::Mat m(long(std::ceil(pixelSize * size.height))
              , long(std::ceil(pixelSize * size.width))
              , CV_8UC1);
    m = cv::Scalar(0);
    auto white(cv::Scalar(0xff));

    mask.forEachQuad([&](uint xstart, uint ystart, uint xsize
                         , uint ysize, bool)
    {
        cv::Point2i start(int(std::floor(pixelSize * xstart))
                          , int(std::floor(pixelSize * ystart)));
        cv::Point2i end(int(std::ceil(pixelSize * (xstart + xsize)))
                        , int(std::ceil(pixelSize * (ystart + ysize))));

        cv::rectangle(m, start, end, white, CV_FILLED, 4);
    }, RasterMask::Filter::white);

    return m;
}

} // namespace quadtree

} // namespace imgproc