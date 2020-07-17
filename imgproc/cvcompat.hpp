/**
 * Copyright (c) 2020 Melown Technologies SE
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

#ifndef imgproc_cvcompat_hpp_included_
#define imgproc_cvcompat_hpp_included_

#include "opencv2/core/version.hpp"

#if CV_VERSION_MAJOR >= 4

// cv::cvtColor constants
#define IMGPROC_CVT_COLOR(what) cv::COLOR_##what

// img codec constants (imread)
#define IMGPROC_IMREAD(what) cv::IMREAD_##what

// contours
#define IMGPROC_CONTOURS(what) cv::what

// distance
#define IMGPROC_DISTANCE(what) cv::what

#else

// cv::cvtColor constants
#define IMGPROC_CVT_COLOR(what) CV_##what

// img codec constants (imread)
#define IMGPROC_IMREAD(what) CV_LOAD_IMAGE_##what

// contours
#define IMGPROC_CONTOURS(what) CV_##what

// distance
#define IMGPROC_DISTANCE(what) CV_##what

#endif

#endif // imgproc_cvcompat_hpp_included_
