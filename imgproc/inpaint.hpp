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
/**
 * @file inpaint.hpp
 * @author Jakub Cerveny <jakub.cerveny@melown.com>
 * @author Matyas Hollmann <matyas.hollmann@melowntech.com>
 *
 * JPEG block inpainting for texture atlas compression
 */

#ifndef imgproc_inpaint_hpp_included_
#define imgproc_inpaint_hpp_included_

#include <opencv2/core/core.hpp>

#include "utility/gccversion.hpp"

#include "rastermask.hpp"

#include "scattered-interpolation.hpp"
#include "utility/openmp.hpp"

namespace imgproc {

#if !defined(IMGPROC_HAS_OPENCV) || !defined(IMGPROC_HAS_EIGEN3)
    UTILITY_FUNCTION_ERROR("JPEG inpaint is available only when compiled with both OpenCV and Eigen3 libraries.")
#endif

/** Fill in pixels in JPEG blocks that have zeros in 'mask', with values
 *  interpolated from neighboring pixels with nonzero 'mask'. Completely
 *  empty blocks are filled with zeros. Completely full blocks are left intact.
 *
 *  typename T_DATA: numeric type of (per-channel) elements of 'img' matrix
 *  int nChan:       number of channels of 'img' matrix
 *
 *  Template default is for 'img' matrix being of type CV_8UC3
 */
template<typename T_DATA = unsigned char, int nChan = 3>
void jpegBlockInpaint(cv::Mat &img, const cv::Mat &mask,
                      int blkWidth = 8, int blkHeight = 8, float eps = 1e-3)
{
    //cv::imwrite("mask.png", mask);
    //cv::imwrite("before.png", img);

    assert(sizeof(T_DATA) == img.elemSize1() && img.channels() == nChan);

    const auto zeroVec = cv::Vec<T_DATA, nChan>();

    UTILITY_OMP(parallel for)
    for (int by = 0; by < img.rows; by += blkHeight)
    {
        imgproc::RasterMask blkMask(blkWidth, blkHeight);
        for (int bx = 0; bx < img.cols; bx += blkWidth)
        {
            int w = std::min(blkWidth,  img.cols - bx);
            int h = std::min(blkHeight, img.rows - by);

            bool full = true, empty = true;

            for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                bool m = mask.at<uchar>(by + y, bx + x);
                if (m) { empty = false; }
                else { full = false; }
                blkMask.set(x, y, m);
            }

            if (full) { continue; }

            cv::Mat block(img, cv::Rect(bx, by, w, h));
            if (!empty) {
                laplaceInterpolate<T_DATA, nChan>(block, blkMask, eps);
            }
            else // make sure empty block is zeroed
            {
                block.setTo(zeroVec);
            }
        }
    }

    //cv::imwrite("inpaint.png", img);
}

} // imgproc

#endif // imgproc_inpaint_hpp_included_
