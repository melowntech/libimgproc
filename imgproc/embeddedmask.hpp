#ifndef imgproc_embeddedmask_hpp_included_
#define imgproc_embeddedmask_hpp_included_

#include <new>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include "./rastermask.hpp"

namespace imgproc {

/** Writes embedded raster mask into existing image file.
 * Supported image format: TIFF
 */
void writeEmbeddedMask(const boost::filesystem::path &imagePath
                       , const quadtree::RasterMask &mask);

/** Reads embedded raster mask from image file.
 * Supported image format: TIFF
 *
 * Throws std::runtime_error if there is no mask present in the file or support
 * for given file type is not available.
 */
quadtree::RasterMask
readEmbeddedMask(const boost::filesystem::path &imagePath);

/** Reads embedded raster mask from image file.
 * Supported image format: TIFF
 *
 * Returns boost::none if there is no mask present in the file or support for
 * given file type is not available.
 */
boost::optional<quadtree::RasterMask>
readEmbeddedMask(const boost::filesystem::path &imagePath
                 , std::nothrow_t);

} // namespace imgproc

#endif // imgproc_embeddedmask_hpp_included_
