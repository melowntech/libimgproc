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
 * @file spectral_analysis.hpp
 * @author Ladislav Horky <ladislav.horky@citationtech.net>
 *
 * Spectral analysis functions
 */

#ifndef imgproc_spectral_analysis_hpp_included_
#define imgproc_spectral_analysis_hpp_included_

#include <opencv2/core/core.hpp>

#include "math/geometry_core.hpp"


namespace imgproc {

void effectiveScale(const cv::Mat & img, float &hscale, float &vscale
                    , float threshold = 0.1);

math::Size2f effectiveScale(const cv::Mat & img, float threshold = 0.1);

// implementation

inline math::Size2f effectiveScale(const cv::Mat & img, float threshold)
{
    float hscale, vscale;
    effectiveScale(img, hscale, vscale, threshold);
    return math::Size2f(hscale, vscale);
}

} //namespace imgproc

#endif //imgproc_spectral_analysis_hpp_included_
