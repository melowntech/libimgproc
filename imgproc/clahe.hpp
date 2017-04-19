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
 * @file clahe.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Contrast-limited adaptive histogram equalization (CLAHE), based on
 * time-proven Karel Zuiderveld 1994 implementation and adapted to support
 * multi-channel arbitrary-dimension OpenCV images.
 */

#ifndef IMGPROC_CLAHE_HPP
#define IMGPROC_CLAHE_HPP

#include <opencv2/core/core.hpp>

namespace imgproc {

/**
 * @brief perform CLAHE on an image
 * @details This function performs contrast-limited adaptive histogram
 * equalization on an input image. clipLimit values lesser then 0 disable
 * contrast limiting and result in the standard AHE algorithm. The input
 * image can be of type CV_8UC1, CV_8UC3, CV_16UC1 or CV_16UC3. If a 3 channel
 * image is supplied, CLAHE is applied to the intensity channel only and
 * chromatic information is left untouched. */
void CLAHE( const cv::Mat & src, cv::Mat & dst, const int regionSize,
            float clipLimit = -1.0 );

} // namespace imgproc

#endif // IMGPROC_CLAHE_HPP
