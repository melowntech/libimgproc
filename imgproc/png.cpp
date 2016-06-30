#include <memory>
#include <algorithm>

#include "dbglog/dbglog.hpp"

#include "./png.hpp"
#include "./error.hpp"

namespace imgproc {

namespace {

extern "C" {

    void imgproc_PngWrite(::png_structp png
                          , ::png_bytep data
                          , ::png_size_t length)
    {
        auto &out(*static_cast<SerializedPng*>(::png_get_io_ptr(png)));
        out.insert(out.end(), data, data + length);
    }

    void imgproc_PngFlush(::png_structp) {}

} // extern "C"

class PngWriter {
public:
    PngWriter(SerializedPng &out)
        : png_(::png_create_write_struct
               (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))
        , info_()
    {
        if (!png_) {
            LOGTHROW(err1, Error)
                << "Unable to initialize PNG writer.";
        }
        info_ = ::png_create_info_struct(png_);

        ::png_set_write_fn(png_
                           , &out
                           , &imgproc_PngWrite
                           , imgproc_PngFlush);
    }

    ~PngWriter() {
        png_destroy_write_struct(&png_, &info_);
    }

    ::png_structp png() { return png_; }
    ::png_infop info() { return info_; }

private:
    ::png_structp png_;
    ::png_infop info_;
};

} // namespace

SerializedPng serialize(const boost::gil::gray8_image_t &image
                        , int compressionLevel)
{
    SerializedPng out;
    PngWriter writer(out);

    ::png_set_IHDR(writer.png(), writer.info(), image.width(), image.height()
                   , 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE
                   , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if ((compressionLevel >= 0) && (compressionLevel <= 9)) {
        ::png_set_compression_level(writer.png(), compressionLevel);
    }

    ::png_write_info(writer.png(), writer.info());

    // copy row by row
    auto view(boost::gil::const_view(image));
    std::vector<boost::gil::gray8_pixel_t> row(view.width());

    for(int y(0), ey(view.height()); y != ey; ++y) {
        std::copy(view.row_begin(y), view.row_end(y), row.begin());
        ::png_write_row(writer.png()
                        , reinterpret_cast< ::png_bytep>(&row.front()));
    }
    ::png_write_end(writer.png(), writer.info());

    return out;
}

} // namespace imgproc
