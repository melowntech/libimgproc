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
#ifndef imgproc_cvmat_hpp_included_
#define imgproc_cvmat_hpp_included_

#include <opencv2/core/core.hpp>

#include "utility/raise.hpp"

#include "error.hpp"

namespace imgproc {

namespace gil = boost::gil;

namespace detail {

template <typename PixelType>
inline void checkMatrixChannels(const cv::Mat &m)
{
    if (gil::num_channels<PixelType>::value != m.channels()) {
        utility::raise<imgproc::TypeError>
            ("Cannot create gil view from OpenCV matrix: matrix "
             "has %d channels but view will have %d channels."
             , m.channels(), gil::num_channels<PixelType>::value);
    }
}

inline const char* cvTypeName(int depth) {
    switch (depth) {
    case CV_8U: return "unsigned char";
    case CV_8S: return "char/signed char";
    case CV_16U: return "unsigned short";
    case CV_16S: return "signed short";
    case CV_32S: return "int";
    case CV_32F: return "float";
    case CV_64F: return "double";
    };
    utility::raise<imgproc::TypeError>
        ("Matrix has unknown value of depth: %s", depth);
    throw;
}

template <typename T> void checkMatrixElement(const cv::Mat &m);

#define GENERATE_CHECK_MATRIX_ELEMENT(TYPE, CV_DEPTH)                   \
    template <>                                                         \
    inline void checkMatrixElement<TYPE>(const cv::Mat &m) {            \
        if (m.depth() != CV_DEPTH) {                                    \
            utility::raise<imgproc::TypeError>                          \
                ("Cannot create gil view from OpenCV matrix: matrix "   \
                 "has different element type (expected %s, got %s)."    \
                 , #TYPE, cvTypeName(m.depth()));                       \
        }                                                               \
    }

GENERATE_CHECK_MATRIX_ELEMENT(unsigned char, CV_8U)
GENERATE_CHECK_MATRIX_ELEMENT(char, CV_8S)
GENERATE_CHECK_MATRIX_ELEMENT(signed char, CV_8S)
GENERATE_CHECK_MATRIX_ELEMENT(unsigned short, CV_16U)
GENERATE_CHECK_MATRIX_ELEMENT(signed short, CV_16S)
GENERATE_CHECK_MATRIX_ELEMENT(signed int, CV_32S)
GENERATE_CHECK_MATRIX_ELEMENT(float, CV_32F)
GENERATE_CHECK_MATRIX_ELEMENT(double, CV_64F)

#undef GENERATE_CHECK_MATRIX_ELEMENT

template <typename PixelType>
inline void checkMatrix(const cv::Mat &m)
{
    checkMatrixChannels<PixelType>(m);
    checkMatrixElement<typename gil::channel_type<PixelType>::type>(m);
}

} // namespace detail

template <typename PixelType>
inline typename gil::image<PixelType, false>::view_t view(cv::Mat &m)
{
    detail::checkMatrix<PixelType>(m);
    return gil::interleaved_view
        (m.cols, m.rows
         , reinterpret_cast<PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
inline typename gil::image<PixelType, false>::const_view_t
view(const cv::Mat &m)
{
    detail::checkMatrix<PixelType>(m);
    return gil::interleaved_view
        (m.cols, m.rows
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
inline typename gil::image<PixelType, false>::const_view_t
const_view(cv::Mat &m)
{
    detail::checkMatrix<PixelType>(m);
    return gil::interleaved_view
        (m.cols, m.rows
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
inline typename gil::image<PixelType, false>::const_view_t
const_view(const cv::Mat &m)
{
    detail::checkMatrix<PixelType>(m);
    return gil::interleaved_view
        (m.cols, m.rows
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

} // namespace imgproc

#endif // imgproc_cvmat_hpp_included_
