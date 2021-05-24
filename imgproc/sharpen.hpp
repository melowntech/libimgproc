/**
 * Copyright (c) 2021 Melown Technologies SE
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
 * @file sharpen.hpp
 * @author Matyas Hollmann <matyas.hollmann@melowntech.com>
 *
 * Image sharpening based on doubleUSM approach
 * (https://cc-extensions.com/products/doubleusm/)
 */

#ifndef imgproc_sharpen_hpp_included_
#define imgproc_sharpen_hpp_included_

#include <opencv2/core/mat.hpp>

namespace imgproc {

struct SharpenParams {
    SharpenParams(float darkAmount = 0, float lightAmount = 0, int kSize = 3,
                  int threshold = 0)
        : darkAmount_(darkAmount), lightAmount_(lightAmount), kSize_(kSize),
          threshold_(threshold)
    {}

    // darkAmount/lightAmount controls dark/light halos
    // usually a good ratio of darkAmount to lightAmount is approximately 2:1
    float darkAmount_,
          lightAmount_;

    // Gaussian kernel size (thresholding and double USM)
    // has to be a positive odd integer
    int kSize_;

    // threshold controls the minimal brightness change that will be sharpened
    // it may be used to prevent relatively smooth ares from becoming 'speckled'
    int threshold_;
};


/**
 * @param img input matrix (currently support only for matrices of depth: CV_8U)
 * @param params parameters controlling sharpening
 * @param isYCrCb Specifies if the input matrix is already in YCrCb color space,
 *                only relevant for a 3-channel input matrix.
 *
 * @throws std::runtime_error in case of an error
 *
 * @return Sharpened image.
*/
cv::Mat sharpen(const cv::Mat& img, const SharpenParams& params, bool isYCrCb);

} // namespace imgproc

#endif // imgproc_sharpen_hpp_included_
