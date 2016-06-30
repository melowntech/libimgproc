#ifndef imgproc_png_hpp_included_
#define imgproc_png_hpp_included_

#include <boost/gil/gil_all.hpp>

#include <png.h>

namespace imgproc {

typedef std::vector<char> SerializedPng;

/** Serialize grayscale GIL image as in-memory PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
SerializedPng serialize(const boost::gil::gray8_image_t &image
                        , int compressionLevel = -1);

} // namespace imgproc

#endif // imgproc_png_hpp_included_
