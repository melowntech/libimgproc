/**
 * Copyright (c) 2019 Melown Technologies SE
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
 * @file imgwarp.hpp
 * @author Pavel Sevecek <pavel.sevecek@melown.com>
 *
 * Image perspective transforms
 */

#ifndef IMGPROC_IMGWARP_HPP
#define IMGPROC_IMGWARP_HPP

#include <opencv2/core/core.hpp>

namespace imgproc {

/**
 * @brief Performs perspective transformation on given image
 * @details This function is a custom reimplementation of cv::warpPerspective,
 * necessary due to bad performance of the OpenCV function with OpenMP parallelization.
 * The results should be identical, except for minor numerical differences.
 *
 * @param src         Input image of type CV_8U
 * @param dst         Output image, resized and filled by the function
 * @param H           3x3 matrix of the transformation
 * @param dsize       Required size of the output image
 * @param border      Specifies handling of pixels outside of the image area.
 *                    Uses values from enum \ref cv::BorderTypes.
 * @param borderValue Value assigned to outside pixels for border mode cv::BORDER_CONSTANT.
 */
void warpPerspective(const cv::Mat& src, cv::Mat& dst, const cv::Mat& H, const cv::Size dsize,
    const int border, const uchar borderValue = 0);

} // namespace imgproc

#endif // IMGPROC_IMGWARP_HPP
