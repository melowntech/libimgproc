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
#ifndef IMGPROC_MORPHOLOGY_HPP
#define IMGPROC_MORPHOLOGY_HPP

#include "math/geometry_core.hpp"

#if IMGPROC_HAS_OPENCV
#include <opencv2/core/core.hpp>
#endif

namespace imgproc {

#if IMGPROC_HAS_OPENCV
template<typename MatType>
void erode(cv::Mat &mat, int kernelSize = 3)
{
    cv::Mat result(mat.rows, mat.cols, mat.type());

    kernelSize /= 2;

    for (int y = 0; y < mat.rows; y++)
    for (int x = 0; x < mat.cols; x++)
    {
        MatType minimum = mat.at<MatType>(y, x);

        for (int i = -kernelSize; i <= kernelSize; i++) {
            if (y+i < 0 || y+i >= mat.rows) continue;

            for (int j = -kernelSize; j <= kernelSize; j++) {
                if (x+j < 0 || x+j >= mat.cols) continue;

                MatType value = mat.at<MatType>(y+i, x+j);
                minimum = std::min(value, minimum);
            }
        }

        result.at<MatType>(y, x) = minimum;
    }

    result.copyTo(mat);
}


template<typename MatType>
void dilate(cv::Mat &mat, int kernelSize = 3)
{
    cv::Mat result(mat.rows, mat.cols, mat.type());

    kernelSize /= 2;

    for (int y = 0; y < mat.rows; y++)
    for (int x = 0; x < mat.cols; x++)
    {
        MatType maximum = mat.at<MatType>(y, x);

        for (int i = -kernelSize; i <= kernelSize; i++) {
            if (y+i < 0 || y+i >= mat.rows) continue;

            for (int j = -kernelSize; j <= kernelSize; j++) {
                if (x+j < 0 || x+j >= mat.cols) continue;

                MatType value = mat.at<MatType>(y+i, x+j);
                maximum = std::max(value, maximum);
            }
        }

        result.at<MatType>(y, x) = maximum;
    }

    result.copyTo(mat);
}
#endif

} // namespace imgproc

#endif // IMGPROC_MORPHOLOGY_HPP