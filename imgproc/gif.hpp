#ifndef imgproc_gif_hpp_included_
#define imgproc_gif_hpp_included_

#include <boost/filesystem/path.hpp>

#include <opencv2/core/core.hpp>

namespace imgproc {

cv::Mat readGif(const void *data, std::size_t size);

cv::Mat readGif(const boost::filesystem::path &path);

} // namespace imgproc

#endif // imgproc_gif_hpp_included_
