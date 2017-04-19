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
#ifndef imgproc_png_hpp_included_
#define imgproc_png_hpp_included_

#include <boost/filesystem/path.hpp>
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

/** Write grayscale GIL image into PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
void write(const boost::filesystem::path &file
           , const boost::gil::gray8_image_t &image
           , int compressionLevel = -1);

/** Write RGB GIL image into PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
void write(const boost::filesystem::path &file
           , const boost::gil::rgb8_image_t &image
           , int compressionLevel = -1);

/** Write RGBA GIL image into PNG file.
 *
 * \param image image to serialize
 * \param compressionLevel 0-9, other values map to default (whatever it is)
 * \return serialized image
 */
void write(const boost::filesystem::path &file
           , const boost::gil::rgba8_image_t &image
           , int compressionLevel = -1);

} } // namespace imgproc::png

#endif // imgproc_png_hpp_included_
