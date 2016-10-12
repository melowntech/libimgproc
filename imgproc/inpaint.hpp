/**
 * @file inpaint.hpp
 * @author Jakub Cerveny <jakub.cerveny@melown.com>
 *
 * JPEG block inpainting for texture atlas compression
 */

#ifndef imgproc_inpaint_hpp_included_
#define imgproc_inpaint_hpp_included_

#include <opencv2/core/core.hpp>

#include "utility/gccversion.hpp"

#include "./rastermask.hpp"

namespace imgproc {

/** Fill in pixels in JPEG blocks that have zeros in 'mask', with values
 *  interpolated from neighboring pixels with nonzero 'mask'. Completely
 *  empty blocks are filled with zeros. Completely full blocks are left intact.
 */
void jpegBlockInpaint(cv::Mat &img, const cv::Mat &mask,
                      int blkWidth = 8, int blkHeight = 8)
#if !defined(IMGPROC_HAS_OPENCV) || !defined(IMGPROC_HAS_EIGEN3)
    UTILITY_FUNCTION_ERROR("JPEG inpaint is available only when compiled with both OpenCV and Eigen3 libraries.")
#endif
    ;
} // imgproc

#endif // imgproc_inpaint_hpp_included_
