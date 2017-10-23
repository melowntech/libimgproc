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
#include <stdexcept>
#include <algorithm>

#include <boost/format.hpp>

#include <gif_lib.h>

#include "dbglog/dbglog.hpp"

#include "./gif.hpp"

namespace imgproc {

namespace {

typedef std::shared_ptr< ::GifFileType> Gif;

Gif openGif(const boost::filesystem::path &path)
{
    auto gif(::DGifOpenFileName(path.c_str()));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF file "
            << path << ": <" << GifLastError() << ".";
    }

    return Gif(gif, [](GifFileType *gif) {
            if (gif) { ::DGifCloseFile(gif); }
        });
}

struct UserData {
    UserData(const void *ptr, int length)
        : ptr(static_cast<const GifByteType*>(ptr)), length(length)
    {}

    int copy(GifByteType *buffer, int l) {
        l = (length < l) ? length : l;

        if (l <= 0) { return 0; }

        std::copy(ptr, ptr + l, buffer);

        ptr += l;
        length -= l;
        return l;
    }

    const GifByteType *ptr;
    int length;
};

extern "C" {

int imgproc_gif_inputfunc(GifFileType *gif, GifByteType *buffer, int length)
{
    auto &userData(*static_cast<UserData*>(gif->UserData));
    return userData.copy(buffer, length);
}

} // extern "C"

Gif openGif(const void *data, std::size_t size)
{
    UserData userData(data, size);
    auto gif(::DGifOpen(&userData, &imgproc_gif_inputfunc));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF from memory: <" << GifLastError() << ".";
    }

    return Gif(gif, [](GifFileType *gif) {
            if (gif) { ::DGifCloseFile(gif); }
        });
}

class Deinterlacer {
public:
    Deinterlacer(int height)
        : height_(height)
        , strips_((height + 7) / 8)
        , pass_(0), passStrips_(0), passLine_(0)

    {}

    int next() {
        auto y(lineIndex());
        if (y >= height_) {
            y = lineIndex();
        }
        return y;
    }

private:
    int lineIndex() {
        auto line(passes[pass_][passLine_]);

        auto y(passStrips_ * 8);
        y += line;

        ++passLine_;

        if (passes[pass_][passLine_] < 0) {
            // next strip
            ++passStrips_;
            passLine_ = 0;
        }

        if (passStrips_ == strips_) {
            ++pass_;
            passStrips_ = 0;
            passLine_ = 0;
        }
        return y;
    };

    const int height_;
    const int strips_;
    int pass_;
    int passStrips_;
    int passLine_;

    static const int *passes[];
};

const int pass1[] = { 0, -1 };
const int pass2[] = { 4, -1 };
const int pass3[] = { 2, 6, -1 };
const int pass4[] = { 1, 3, 5, 7, -1 };

const int *Deinterlacer::passes[] = { pass1, pass2, pass3, pass4 };

cv::Mat readGif(GifFileType *gif, const std::string &source)
{
#define CHECK_GIF_STATUS(what)                                          \
    do {                                                                \
        if (what == GIF_ERROR) {                                        \
            PrintGifError();                                            \
            LOGTHROW(err1, std::runtime_error)                          \
                << "Failed to process gif " << source                   \
                << ": <" << GifLastError() << ">.";                     \
        }                                                               \
    } while (0)

    CHECK_GIF_STATUS(::DGifSlurp(gif));

    cv::Mat out(gif->SHeight, gif->SWidth, CV_8UC3);

    Deinterlacer deinterlacer(gif->Image.Height);

    unsigned char *rb(gif->SavedImages[0].RasterBits);
    for (int j = 0; j < gif->Image.Height; ++j) {
        auto y(gif->Image.Top
               + (gif->Image.Interlace ? deinterlacer.next() : j));

        for (int i = 0; i < gif->Image.Width; ++i) {
            auto colorIndex(*rb++);

            // const auto &color(gif->Image.ColorMap->Colors[colorIndex]);
            const auto &color(gif->SColorMap->Colors[colorIndex]);

            auto x = 3 * (i + gif->Image.Left);

            out.at<unsigned char>(y, x + 0) = color.Blue;
            out.at<unsigned char>(y, x + 1) = color.Green;
            out.at<unsigned char>(y, x + 2) = color.Red;
        }
    }

#undef CHECK_GIF_STATUS
    return out;
}

} // namesapce

cv::Mat readGif(const void *data, std::size_t size)
{
    auto gif(openGif(data, size));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF from memory: <"
            << GifLastError() << ">.";
    }
    return readGif(gif.get(), "from memory");
}

cv::Mat readGif(const boost::filesystem::path &path)
{
    auto gif(openGif(path));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF file "
            << path << ": <" << GifLastError() << ".";
    }
    return readGif(gif.get(), str(boost::format("file %s") % path));
}

math::Size2 gifSize(const boost::filesystem::path &path)
{
    auto gif(openGif(path));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF file "
            << path << ": <" << GifLastError() << ".";
    }
    return { int(gif->SWidth), int(gif->SHeight) };
}

math::Size2 gifSize(const void *data, std::size_t size)
{
    auto gif(openGif(data, size));
    if (!gif) {
        LOGTHROW(err1, std::runtime_error)
            << "Failed to open GIF from memory: <"
            << GifLastError() << ">.";
    }
    return { int(gif->SWidth), int(gif->SHeight) };
}

} // namespace imgproc
