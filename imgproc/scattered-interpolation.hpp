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
 * @file scattered-interpolation.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Interpolation on 2D scattered data.
 */

#ifndef imgproc_scattered_interpolation_hpp_included_
#define imgproc_scattered_interpolation_hpp_included_

#include <opencv2/core/core.hpp>

#include "utility/gccversion.hpp"

#include "rastermask.hpp"

namespace imgproc {

/** Solves the boundary value problem -\Delta u = 0 on elements in the matrix
 *  'data' that correspond to unset elements in 'mask'. Elements corresponding
 *  to set positions in 'mask' are regarded as given data.
 *
 *  The method is described in section 3.8 "Laplace Interpolation" of
 *  Numerical Recipes in C, Third Edition.
 *
 *  NOTE: 'data' must be of type CV_32F.
 */
void laplaceInterpolate(cv::Mat &data, const imgproc::RasterMask &mask,
                        double tol = 1e-12);
#if !defined(IMGPROC_HAS_OPENCV) || !defined(IMGPROC_HAS_EIGEN3)
    UTILITY_FUNCTION_ERROR("Laplace interpolation is available only when compiled with both OpenCV and Eigen3 libraries.")
#endif

} // imgproc

#endif // imgproc_scattered_interpolation_hpp_included_
