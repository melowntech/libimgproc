#ifndef imgproc_png_hpp_included_
#define imgproc_png_hpp_included_

#include <boost/gil/gil_all.hpp>

#include <png.h>

namespace imgproc {

typedef std::vector<char> SerializedPng;

SerializedPng serialize(const boost::gil::gray8_image_t &image);

} // namespace imgproc

#endif // imgproc_png_hpp_included_
