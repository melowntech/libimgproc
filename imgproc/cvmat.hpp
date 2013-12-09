#ifndef imgproc_cvmat_hpp_included_
#define imgproc_cvmat_hpp_included_

#include <opencv2/core/mat.hpp>

#include "error.hpp"

#include "gil-float-image.hpp"

namespace imgproc {

namespace gil = boost::gil;

gil::gray_float_view_t grayFloatView(cv::Mat &m)
{
    if (m.type() != CV_32FC1) {
        throw TypeError("OpenCV matrix is not one channel float.");
    }
    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<gil::gray_float_pixel_t*>(m.data)
         , m.step);
}

gil::gray_float_const_view_t grayFloatView(const cv::Mat &m)
{
    if (m.type() != CV_32FC1) {
        throw TypeError("OpenCV matrix is not one channel float.");
    }

    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<const gil::gray_float_pixel_t*>(m.data)
         , m.step);
}

gil::gray_float_const_view_t grayFloatConstView(cv::Mat &m)
{
    return grayFloatView(static_cast<const cv::Mat&>(m));
}

gil::gray_float_const_view_t grayFloatConstView(const cv::Mat &m)
{
    return grayFloatView(m);
}


} // namespace imgproc

#endif // imgproc_cvmat_hpp_included_
