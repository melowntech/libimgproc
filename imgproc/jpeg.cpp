#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

#include "dbglog/dbglog.hpp"

#include "./error.hpp"
#include "./jpeg.hpp"

namespace fs = boost::filesystem;

namespace imgproc {

math::Size2 jpegSize(std::istream &is, const fs::path &path)
{
    char buf[1024];
    std::size_t size(sizeof(buf));

    {
        auto exc(utility::scopedStreamExceptions(is));

        // clear EOF bit
        is.exceptions(exc.state()
                      & ~(std::ios_base::failbit | std::ios_base::eofbit));
        size = is.read(buf, size).gcount();
        is.clear();
    }

    return jpegSize(buf, size, path);
}

math::Size2 jpegSize(const void *data, std::size_t size
                     , const fs::path &path)
{
    struct Decompressor {
        Decompressor()
            : init_(false)
        {
            cinfo.err = ::jpeg_std_error(&jerr_);
            ::jpeg_create_decompress(&cinfo);
            init_ = true;
        }

        ~Decompressor() {
            if (init_) {
                ::jpeg_destroy_decompress(&cinfo);
            }
        }

        ::jpeg_decompress_struct cinfo;

    private:
        bool init_;
        ::jpeg_error_mgr jerr_;
    } dc;

    ::jpeg_mem_src(&dc.cinfo, (unsigned char*)(data), size);
    auto res(::jpeg_read_header(&dc.cinfo, TRUE));

    if (res != JPEG_HEADER_OK) {
        LOGTHROW(err4, std::runtime_error)
            << "Unable to determine size of JPEG " << path << ".";
    }

    // fine
    return math::Size2(dc.cinfo.image_width, dc.cinfo.image_height);
}

} // namespace imgproc
