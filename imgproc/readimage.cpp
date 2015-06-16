#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include <boost/algorithm/string/case_conv.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"

#include "./readimage.hpp"
#include "./error.hpp"
#include "./jp2.hpp"

#ifdef IMGPROC_HAS_GIF
#  include "./gif.hpp"
#endif

#ifdef IMGPROC_HAS_TIFF
#  include "./tiff.hpp"
#endif

namespace gil = boost::gil;
namespace ba = boost::algorithm;

namespace imgproc {

cv::Mat readImage(const void *data, std::size_t size)
{
    auto image(cv::imdecode({data, int(size)}, CV_LOAD_IMAGE_COLOR));

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

cv::Mat readImage(const boost::filesystem::path &path)
{
    std::string ext(path.extension().string());
    ba::to_lower(ext);

#ifdef IMGPROC_HAS_TIFF
    // TIFF-specific read
    if (ext == ".tif") {
        try {
            auto image(imgproc::readTiff(path));
            if (image.data) { return image; }
        } catch (const std::runtime_error &e) {}
    }
#endif

#ifdef IMGPROC_HAS_GIF
    if (ext == ".gif") {
        try {
            auto image(imgproc::readGif(path));
            if (image.data) { return image; }
        } catch (const std::runtime_error &e) {}
    }
#endif

    // generic read
    return cv::imread(path.string(), CV_LOAD_IMAGE_COLOR);
}

math::Size2 imageSize(const boost::filesystem::path &path)
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

} // namespace imgproc
