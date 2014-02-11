#include "./cvmat.hpp"

namespace imgproc {

namespace detail {

template <typename Mask>
inline cv::Mat asCvMat(const Mask &mask, int pixelSize)
{
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
}

} // namespace detail

namespace bitfield {

cv::Mat asCvMat(const RasterMask &mask, int pixelSize)
{
    return detail::asCvMat(mask, pixelSize);
}

} // namespace bitfield

namespace quadtree {

cv::Mat asCvMat(const RasterMask &mask, int pixelSize)
{
    return detail::asCvMat(mask, pixelSize);
}

} // namespace quadtree

} // namespace imgproc
