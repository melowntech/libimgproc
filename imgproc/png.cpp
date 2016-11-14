#include <cstdio>
#include <memory>
#include <algorithm>
#include <system_error>

#include <png.h>

#include "dbglog/dbglog.hpp"

#include "./png.hpp"
#include "./error.hpp"

namespace fs = boost::filesystem;

namespace imgproc { namespace png {

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

    PngWriter(FILE *out)
        : png_(::png_create_write_struct
               (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))
        , info_()
    {
        if (!png_) {
            LOGTHROW(err1, Error)
                << "Unable to initialize PNG writer.";
        }
        info_ = ::png_create_info_struct(png_);

        ::png_init_io(png_, out);
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

template <typename PixelType, typename ConstView>
void writeView(PngWriter &writer, const ConstView &view
               , int compressionLevel, int type)
{
    ::png_set_IHDR(writer.png(), writer.info(), view.width(), view.height()
                   , 8, type, PNG_INTERLACE_NONE
                   , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if ((compressionLevel >= 0) && (compressionLevel <= 9)) {
        ::png_set_compression_level(writer.png(), compressionLevel);
    }

    ::png_write_info(writer.png(), writer.info());

    std::vector<PixelType> row(view.width());

    // copy row by row
    for(int y(0), ey(view.height()); y != ey; ++y) {
        std::copy(view.row_begin(y), view.row_end(y), row.begin());
        ::png_write_row(writer.png()
                        , reinterpret_cast< ::png_bytep>(&row.front()));
    }

    ::png_write_end(writer.png(), writer.info());
}

template <typename PixelType, typename ConstView>
SerializedPng serializeView(const ConstView &view, int compressionLevel
                            , int type)
{
    SerializedPng out;
    PngWriter writer(out);
    writeView<PixelType>(writer, view, compressionLevel, type);
    return out;
}

template <typename PixelType, typename ConstView>
void writeViewToFile(const fs::path &path, const ConstView &view
                     , int compressionLevel, int type)
{
    struct FileHolder {
        FileHolder(const fs::path &path)
            : path(path), f(std::fopen(path.c_str(), "w"))
        {}

        ~FileHolder() {
            if (!f) { return; }
            if (std::fclose(f)) {
                std::system_error e(errno, std::system_category());
                LOG(warn3) << "Cannot close PNG file " << path << ": <"
                           << e.code() << ", " << e.what() << ">.";
            }
        }

        const fs::path path;
        std::FILE *f;
    };

    FileHolder fh(path);
    if (!fh.f) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot create PNG file " << path << ": <"
                   << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    PngWriter writer(fh.f);
    writeView<PixelType>(writer, view, compressionLevel, type);
}

} // namespace

SerializedPng serialize(const boost::gil::gray8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::gray8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_GRAY);
}

void write(const fs::path &file
           , const boost::gil::gray8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::gray8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_GRAY);
}

SerializedPng serialize(const boost::gil::rgb8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::rgb8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_RGB);
}

void write(const fs::path &file
           , const boost::gil::rgb8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::rgb8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_RGB);
}

SerializedPng serialize(const boost::gil::rgba8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::rgba8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_RGBA);
}


void write(const fs::path &file
           , const boost::gil::rgba8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::rgba8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_RGBA);
}

} } // namespace imgproc::png
