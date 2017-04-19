/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
