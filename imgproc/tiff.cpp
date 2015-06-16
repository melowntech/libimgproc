#include <boost/gil/extension/io/tiff_io.hpp>

#include "dbglog/dbglog.hpp"

#include "./tiff.hpp"
#include "./error.hpp"
#include "./cvmat.hpp"

namespace imgproc {

namespace gil = boost::gil;
namespace fs = boost::filesystem;

namespace detail {

typedef boost::shared_ptr<TIFF> Tiff;

Tiff openTiff(const fs::path &path)
{
    if (auto t = TIFFOpen(path.string().c_str(), "r")) {
        return Tiff(t, [](TIFF *t) { if (t) TIFFClose(t); });
    }

    LOGTHROW(err1, Error)
        << "Cannot open TIFF file " << path << ".";
    throw;
}

struct ImageParams {
    fs::path path;

    std::uint16_t bpp;
    std::uint16_t orientation;
    std::uint32_t width;
    std::uint32_t height;


    ImageParams(const fs::path &path)
        : path(path), bpp(8), orientation(1), width(0), height(0)
    {}

    int cvType() const {
        switch (bpp) {
        case 8: return CV_8UC3;
        case 16: return CV_16UC3;
        }

        LOGTHROW(err1, Error)
            << "Unsupported bit field " << bpp
            << " in  TIFF file " << path << ".";
        throw;
    }
};

ImageParams getParams(const fs::path &path)
{
    auto tiff(openTiff(path));
    ImageParams params(path);
    if (!TIFFGetField(tiff.get(), TIFFTAG_BITSPERSAMPLE, &params.bpp)) {
        params.bpp = 8;
    }

    if (!TIFFGetField(tiff.get(), TIFFTAG_ORIENTATION, &params.orientation)) {
        params.orientation = 1;
    }

    if (!TIFFGetField(tiff.get(), TIFFTAG_IMAGEWIDTH, &params.width)) {
        LOGTHROW(err1, Error)
            << "Cannot get TIFF file " << path << " width.";
    }
    if (!TIFFGetField(tiff.get(), TIFFTAG_IMAGELENGTH, &params.height)) {
        LOGTHROW(err1, Error)
            << "Cannot get TIFF file " << path << " height.";
    }

    return params;
}

} // namespace detail;

cv::Mat readTiff(const void *data, std::size_t size)
{
    (void) data;
    (void) size;
    LOGTHROW(err3, Error)
        << "Open TIFF from memory: not-implemented.";
    throw;
}

cv::Mat readTiff(const fs::path &path)
{
    auto params(detail::getParams(path));

    cv::Mat img(params.height, params.width, params.cvType());

    switch (params.bpp) {
    case 8:
        gil::tiff_read_and_convert_view
            (path.c_str(), imgproc::view<gil::bgr8_pixel_t>(img));
        break;

    case 16:
        gil::tiff_read_and_convert_view
            (path.c_str(), imgproc::view<gil::bgr16_pixel_t>(img));
        break;
    }

    return img;
}

math::Size2 tiffSize(const fs::path &path)
{
    auto size(gil::tiff_read_dimensions(path.string()));
    return { int(size.x), int(size.y) };
}

} // namespace imgproc
