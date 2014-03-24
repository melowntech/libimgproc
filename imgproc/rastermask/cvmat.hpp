#ifndef imgproc_rastermask_cvmat_hpp_included_
#define imgproc_rastermask_cvmat_hpp_included_

#include <opencv2/core/core.hpp>

#include "./bitfield.hpp"
#include "./quadtree.hpp"

namespace imgproc {

namespace bitfield {

cv::Mat asCvMat(const RasterMask &mask, double pixelSize = 1);

} // namespace bitfield

namespace quadtree {

cv::Mat asCvMat(const RasterMask &mask, double pixelSize = 1);

} // namespace quadtree

} // namespace imgproc

#endif // imgproc_rastermask_cvmat_hpp_included_
