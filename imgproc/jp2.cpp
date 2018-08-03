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

#ifdef _WIN32
#  include <Winsock2.h>
#else
#  include <arpa/inet.h>
#endif

#include <fstream>

#include "dbglog/dbglog.hpp"

#include "utility/binaryio.hpp"

#include "./error.hpp"
#include "./jp2.hpp"

namespace imgproc {

namespace bio = utility::binaryio;

namespace jp2 {
    const std::uint32_t Signature(0x6a502020);

    const std::uint8_t Magic[4] = { 0x0d, 0x0a, 0x87, 0x0a };

    const std::uint32_t FileType(0x66747970);
    const std::uint32_t Header(0x6a703268);
    const std::uint32_t ImageHader(0x69686472);

    struct Box {
        std::uint32_t size;
        std::uint32_t type;
        std::vector<std::uint8_t> data;

        void readData(std::istream &is) {
            // skip reading data of super header
            if (type != Header) {
                data.resize(size - 8);
                bio::read(is, data.data(), data.size());
            }
        }
    };

    Box readBoxHeader(std::istream &is)
    {
        Box box;
        bio::read(is, box.size);
        bio::read(is, box.type);
        box.size = ntohl(box.size);
        box.type = ntohl(box.type);
        return box;
    }

    Box readBox(std::istream &is)
    {
        auto box(readBoxHeader(is));
        box.readData(is);
        return box;
    }

    std::uint32_t get32(const void *raw) {
        return ntohl(*static_cast<const std::uint32_t*>(raw));
    }

    std::uint32_t get32(const Box &box, unsigned int index) {
        return get32(&box.data[index]);
    }
}

math::Size2 jp2Size(std::istream &is, const boost::filesystem::path &path)
{
    {
        auto signature(jp2::readBoxHeader(is));
        if (signature.type != jp2::Signature) {
            LOGTHROW(err1, Error)
                << "Not a JP2 file: " << path << ": expected signature box.";
        }
        signature.readData(is);

        if (!std::equal(signature.data.begin(), signature.data.end()
                        , jp2::Magic))
        {
            LOGTHROW(err1, Error)
                << "Not a JP2 file: " << path << ": invalid magic.";
        }
    }

    if (jp2::readBox(is).type != jp2::FileType) {
        LOGTHROW(err1, Error)
            << "Not a JP2 file: " << path << ": expected file type box.";
    }

    if (jp2::readBox(is).type != jp2::Header) {
        LOGTHROW(err1, Error)
            << "Not a JP2 file: " << path << ": expected header box.";
    }

    // search for image header box
    for (int leftBoxes(10); leftBoxes; --leftBoxes) {
        auto box(jp2::readBox(is));
        if (box.type == jp2::ImageHader) {
            return { int(get32(box, 4)),  int(get32(box, 0)) };
        }
    }

    LOGTHROW(err1, Error)
        << "Not a JP2 file: " << path << ": unable to find image header.";
    throw;
}

math::Size2 jp2Size(const boost::filesystem::path &path)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);

    return jp2Size(f, path);
}

} // namespace imgproc
