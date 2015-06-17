#ifndef imgproc_readimage_hpp_included_
#define imgproc_readimage_hpp_included_

#include <boost/filesystem/path.hpp>

#include <opencv2/core/core.hpp>

#include "math/geometry_core.hpp"

namespace imgproc {

cv::Mat readImage(const void *data, std::size_t size);

cv::Mat readImage(const boost::filesystem::path &path);

cv::Mat readImage8bit(const boost::filesystem::path &path);

math::Size2 imageSize(const boost::filesystem::path &path);

} // namespace imgproc

#endif // imgproc_readimage_hpp_included_
