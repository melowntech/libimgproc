#ifndef imgproc_jpeg_hpp_included_
#define imgproc_jpeg_hpp_included_

#include <iostream>
#include <boost/filesystem/path.hpp>

#include "math/geometry_core.hpp"

namespace imgproc {

math::Size2 jpegSize(std::istream &is
                     , const boost::filesystem::path &path = "unknown");

math::Size2 jpegSize(const void *data, std::size_t size
                     , const boost::filesystem::path &path = "unknown");

} // namespace imgproc

#endif // imgproc_jpeg_hpp_included_
