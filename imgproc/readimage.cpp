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
#include <boost/gil/extension/io/jpeg_io.hpp>

#include <boost/algorithm/string/case_conv.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"

#include "./readimage.hpp"
#include "./error.hpp"
#include "./jp2.hpp"
#include "./png_io.hpp"

#ifdef IMGPROC_HAS_GIF
#  include "./gif.hpp"
#endif

#ifdef IMGPROC_HAS_TIFF
#  include "./tiff.hpp"
#endif

#include "./png.hpp"

#ifdef IMGPROC_HAS_JPEG
#  include "./jpeg.hpp"
#endif

namespace gil = boost::gil;
namespace ba = boost::algorithm;
namespace fs = boost::filesystem;

namespace imgproc {

cv::Mat readImage(const void *data, std::size_t size)
{
    auto image(cv::imdecode({data, int(size)}
               , CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYDEPTH));

#ifdef IMGPROC_HAS_GIF
    if (!image.data) {
        // try gif
        try {
            image = imgproc::readGif(data, size);
        } catch (const std::runtime_error &e) {
        }
    }
#endif

    // TODO: try tiff
    return image;
}

cv::Mat readImage(const fs::path &path)
{
    std::string ext(path.extension().string());
    ba::to_lower(ext);

#ifdef IMGPROC_HAS_TIFF
    // TIFF-specific read
    if (ext == ".tif") {
        try {
            auto image(imgproc::readTiff(path));
            if (image.data) { return image; }
            LOG(warn1) << "TIFF-specific reader failed with an unknown error; "
                "trying generic OpenCV-provided loader.";
        } catch (const std::exception &e) {
            LOG(warn1) << "TIFF-specific reader failed with <"
                       << e.what() << ">; trying generic OpenCV-provided "
                "loader.";
        }
    }
#endif

#ifdef IMGPROC_HAS_GIF
    if (ext == ".gif") {
        try {
            auto image(imgproc::readGif(path));
            if (image.data) { return image; }
            LOG(warn1) << "GIF-specific reader failed with an unknown error; "
                "trying generic OpenCV-provided loader.";
        } catch (const std::exception &e) {
            LOG(warn1) << "GIF-specific reader failed with <"
                       << e.what() << ">; trying generic OpenCV-provided "
                "loader.";
        }
    }
#endif

    // generic read
    return cv::imread(path.string()
                      , CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYDEPTH);
}

cv::Mat readImage8bit(const fs::path &path)
{
    auto image(readImage(path));
    if (image.depth() == CV_16U) {
        // convert to 8 bits
        cv::Mat tmp;
        image.convertTo(tmp, CV_8U, 255.0 / 65535.0);
        return tmp;
    }
    return image;
}

math::Size2 imageSize(const fs::path &path)
{
    std::string ext(path.extension().string());
    ba::to_lower(ext);

    if ((ext == ".jpg") || (ext == ".jpeg")) {
#ifdef IMGPROC_HAS_JPEG
        auto size(gil::jpeg_read_dimensions(path.string()));
        return { int(size.x), int(size.y) };
#else
    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": JPEG support not compiled in.";
#endif
    }

    if (ext == ".tif") {
#ifdef IMGPROC_HAS_TIFF
        return tiffSize(path);
#else
    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": TIFF support not compiled in.";
#endif
    }

    if (ext == ".png") {
#ifdef IMGPROC_HAS_PNG
        auto size(gil::png_read_dimensions(path.string()));
        return { int(size.x), int(size.y) };
#else
    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": PNG support not compiled in.";
#endif
    }

    if (ext == ".jp2") {
        return jp2Size(path);
    }

    if (ext == ".gif") {
#ifdef IMGPROC_HAS_GIF
        return gifSize(path);
#else
    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": GIF support not compiled in.";
#endif
    }

    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": Unknown file format.";
    throw;
}

math::Size2 imageSize(std::istream &is, const fs::path &path)
{
    // call format-based measuring function based on first byte of data; we do
    // not check full magic because stream can be non-seekable

    // peek at first byte
    const auto head(is.peek());

    switch (head) {
    case 0xff:
#ifdef IMGPROC_HAS_JPEG
        // looks like JPEG
        return jpegSize(is, path);
#else
        LOGTHROW(err1, Error)
            << "Cannot determine size of image in file " << path
            << ": JPEG support not compiled in.";
        break;
#endif

    case 0x89:
        return png::size(is, path);

    case 'I': case 'M':
#ifdef IMGPROC_HAS_TIFF
        LOGTHROW(err1, Error)
            << "FIXME: Cannot determine size of image in file " << path
            << ": stream-based TIFF image measurement not implemented yet.";
#else
        LOGTHROW(err1, Error)
            << "Cannot determine size of image in file " << path
            << ": TIFF support not compiled in.";
        break;
#endif

        case 0x47:
#ifdef IMGPROC_HAS_GIF
        LOGTHROW(err1, Error)
            << "FIXME: Cannot determine size of image in file " << path
            << ": stream-based GIF image measurement not implemented yet.";
#else
        LOGTHROW(err1, Error)
            << "Cannot determine size of image in file " << path
            << ": GIF support not compiled in.";
        break;
#endif

    case 0x00:
        return jp2Size(is, path);
    }

    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": Unknown file format.";
    throw;
}

std::string imageType(std::istream &is, const fs::path &path)
{
    // TODO: do more peeks

    // peek at first byte
    const auto head(is.peek());

    switch (head) {
    case 0xff: return ".jpg";
    case 0x89: return ".png";

    case 'I': case 'M': return ".tif";
    case 0x47: return ".gif";
    case 0x00: return ".jp2"; // a bit stretch...
    }

    LOGTHROW(err1, Error)
            << "Cannot determine type of image in file " << path
            << ": Unknown file format.";
    throw;
}

} // namespace imgproc
