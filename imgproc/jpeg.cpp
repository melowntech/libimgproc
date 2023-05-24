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
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

#include "dbglog/dbglog.hpp"

#include "error.hpp"
#include "jpeg.hpp"

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
    auto res(::jpeg_read_header(&dc.cinfo, static_cast<boolean>(true)));

    if (res != JPEG_HEADER_OK) {
        LOGTHROW(err4, std::runtime_error)
            << "Unable to determine size of JPEG " << path << ".";
    }

    // fine
    return math::Size2(dc.cinfo.image_width, dc.cinfo.image_height);
}

} // namespace imgproc
