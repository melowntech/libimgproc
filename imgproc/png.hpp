#ifndef imgproc_png_hpp_included_
#define imgproc_png_hpp_included_

#include <boost/gil/gil_all.hpp>

namespace imgproc { namespace png {

typedef std::vector<char> SerializedPng;

/** Serialize grayscale GIL image as in-memory PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
SerializedPng serialize(const boost::gil::gray8_image_t &image
                        , int compressionLevel = -1);

/** Serialize RGB GIL image as in-memory PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
SerializedPng serialize(const boost::gil::rgb8_image_t &image
                        , int compressionLevel = -1);

/** Serialize RGBA GIL image as in-memory PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
SerializedPng serialize(const boost::gil::rgba8_image_t &image
                        , int compressionLevel = -1);

} } // namespace imgproc::png

#endif // imgproc_png_hpp_included_
