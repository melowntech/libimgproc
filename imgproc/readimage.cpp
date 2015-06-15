#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/extension/io/tiff_io.hpp>

#include <boost/algorithm/string/case_conv.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"

#include "./readimage.hpp"
#include "./error.hpp"

#ifdef IMGPROC_HAS_GIF
#  include "./gif.hpp"
#endif

#ifdef IMGPROC_HAS_JASPER
#  include <jasper/jasper.h>
#endif

namespace gil = boost::gil;
namespace ba = boost::algorithm;

namespace imgproc {

namespace detail {

#ifdef IMGPROC_HAS_JASPER
typedef std::shared_ptr< ::jas_stream_t> JPStream;
typedef std::shared_ptr< ::jas_image_t> JPImage;

JPStream openJPStream(const boost::filesystem::path &path)
{
    auto jp(::jas_stream_fopen(path.c_str(), "rb" ));
    if (!jp) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open JP2 file "
            << path << ".";
    }

    return JPStream(jp, [](::jas_stream_t *jp) {
            if (jp) { ::jas_stream_close(jp); }
        });
}

JPImage decodeJPImage(const boost::filesystem::path &path
                      , const JPStream &stream)
{
    auto i(::jas_image_decode(stream.get(), -1, nullptr));
    if (!i) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to decode JP2 image "
            << path << ".";
    }

    return JPImage(i, [](::jas_image_t *i) {
            if (i) { ::jas_image_destroy(i); }
        });
}

math::Size2 jp2Size(const boost::filesystem::path &path)
{
    auto stream(openJPStream(path));
    auto image(decodeJPImage(path, stream));

    return { int(jas_image_width(image.get()))
            , int(jas_image_height(image.get())) };
}
#endif
} // namespace detail

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
    auto image(cv::imread(path.string(), CV_LOAD_IMAGE_COLOR));

#ifdef IMGPROC_HAS_GIF
    if (!image.data) {
        // try gif
        try {
            image = imgproc::readGif(path);
        } catch (const std::runtime_error &e) {
        }
    }
#endif

    // TODO: try tiff
    return image;
}

math::Size2 imageSize(const boost::filesystem::path &path)
{
    std::string ext(path.extension().string());
    ba::to_lower(ext);

    if ((ext == ".jpg") || (ext == ".jpeg")) {
        auto size(gil::jpeg_read_dimensions(path.string()));
        return { int(size.x), int(size.y) };
    }

    if (ext == ".tif") {
        auto size(gil::tiff_read_dimensions(path.string()));
        return { int(size.x), int(size.y) };
    }

    if (ext == ".png") {
        auto size(gil::png_read_dimensions(path.string()));
        return { int(size.x), int(size.y) };
    }

    if (ext == ".jp2") {
#ifdef IMGPROC_HAS_JASPER
        return detail::jp2Size(path);
#else
    LOGTHROW(err1, Error)
        << "Cannot determine size of image in file " << path
        << ": JPEG2000 support not compiled in.";
#endif
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