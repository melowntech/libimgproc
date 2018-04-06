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

#include <cstdio>
#include <memory>
#include <algorithm>
#include <system_error>

#include "dbglog/dbglog.hpp"

#include "utility/binaryio.hpp"

#include "./png.hpp"
#include "./error.hpp"

namespace fs = boost::filesystem;
namespace bin = utility::binaryio;

namespace imgproc { namespace png {

namespace constants {

const unsigned char Signature[8] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1A, 0x0a
};
const unsigned char IHDR[4] = { 'I', 'H', 'D', 'R' };

} // namespace constants

math::Size2 size(std::istream &is, const fs::path &path)
{
    // load header
    unsigned char magic[sizeof(constants::Signature)];
    bin::read(is, magic);

    if (std::memcmp(magic, constants::Signature, sizeof(constants::Signature)))
    {
        LOGTHROW(err1, FormatError)
            << "File " << path << " is not a PNG file.";
    }

    // load and decode IHDR
    const auto length(ntohl(bin::read<std::uint32_t>(is)));

    if (length != 13) {
        LOGTHROW(err1, FormatError)
            << "No IHDR found after header in PNG file " << path << ".";
    }

    unsigned char type[sizeof(constants::IHDR)];
    bin::read(is, type);

    if (std::memcmp(type, constants::IHDR, sizeof(constants::IHDR))) {
        LOGTHROW(err1, FormatError)
            << "No IHDR found after header in PNG file " << path << ".";
    }

    const auto width(ntohl(bin::read<std::uint32_t>(is)));
    const auto height(ntohl(bin::read<std::uint32_t>(is)));

    return math::Size2(width, height);
}

math::Size2 size(const void *data, std::size_t dataSize, const fs::path &path)
{
    (void) data;
    (void) dataSize;
    (void) path;
    return {};
}

} } // namespace imgproc::png
