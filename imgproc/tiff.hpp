#ifndef imgproc_tiff_hpp_included_
#define imgproc_tiff_hpp_included_

#include <boost/filesystem/path.hpp>

#include <opencv2/core/core.hpp>

#include "math/geometry_core.hpp"

namespace imgproc {

cv::Mat readTiff(const void *data, std::size_t size);

cv::Mat readTiff(const boost::filesystem::path &path);

math::Size2 tiffSize(const boost::filesystem::path &path);

} // namespace imgproc

#endif // imgproc_tiff_hpp_included_
