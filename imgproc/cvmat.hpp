#ifndef imgproc_cvmat_hpp_included_
#define imgproc_cvmat_hpp_included_

#include <opencv2/core/mat.hpp>

#include "utility/raise.hpp"

#include "error.hpp"
#include "gil-float-image.hpp"

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
