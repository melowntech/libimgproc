#ifndef imgproc_cvmat_hpp_included_
#define imgproc_cvmat_hpp_included_

#include <opencv2/core/mat.hpp>

#include "error.hpp"

#include "gil-float-image.hpp"

namespace imgproc {

namespace gil = boost::gil;

template <typename PixelType>
typename gil::image<PixelType, false>::view_t view(cv::Mat &m)
{
    // TODO: check matrix type/channels

    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
typename gil::image<PixelType, false>::const_view_t view(const cv::Mat &m)
{
    // TODO: check matrix type/channels

    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
typename gil::image<PixelType, false>::const_view_t const_view(cv::Mat &m)
{
    // TODO: check matrix type/channels

    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

template <typename PixelType>
typename gil::image<PixelType, false>::const_view_t
const_view(const cv::Mat &m)
{
    // TODO: check matrix type/channels

    return gil::interleaved_view
        (m.rows, m.cols
         , reinterpret_cast<const PixelType*>(m.data)
         , m.step);
}

} // namespace imgproc

#endif // imgproc_cvmat_hpp_included_
