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
/**
 * @file bitfield.cpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#include <iostream>
#include <stdexcept>
#include <numeric>
#include <cmath>

#include "dbglog/dbglog.hpp"
#include "utility/binaryio.hpp"
#include "math/math.hpp"

#include "./bitfield.hpp"

namespace imgproc { namespace bitfield {

namespace {
    const char BF_RASTERMASK_IO_MAGIC[5] = { 'R', 'M', 'A', 'S', 'K' };

    using utility::binaryio::read;
    using utility::binaryio::write;
}

void RasterMask::dump(std::ostream &f) const
{
    write(f, BF_RASTERMASK_IO_MAGIC); // 5 bytes
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved

    uint32_t width(size_.width);
    uint32_t height(size_.height);
    write(f, width);
    write(f, height);

    write(f, mask_.get(), bytes_);
}

void RasterMask::load(std::istream &f)
{
    char magic[5];
    read(f, magic);

    if (std::memcmp(magic, BF_RASTERMASK_IO_MAGIC,
                    sizeof(BF_RASTERMASK_IO_MAGIC))) {
        LOGTHROW(err2, std::runtime_error) << "RasterMask has wrong magic.";
    }

    uint8_t reserved1, reserved2, reserved3;
    read(f, reserved1); // reserved
    read(f, reserved2); // reserved
    read(f, reserved3); // reserved

    uint32_t width, height;
    read(f, width);
    read(f, height);

    size_.width = width;
    size_.height = height;
    bytes_ = (size_.height * size_.width + 7) >> 3;
    mask_.reset(new std::uint8_t[bytes_]);

    read(f, mask_.get(), bytes_);

    resetTrail();

    // compute count_
    count_ = std::accumulate
        (mask_.get(), mask_.get() + bytes_, 0u
         , [](unsigned int value, std::uint8_t byte) {
            return (value + (byte & 0x01) + ((byte & 0x02) >> 1)
                    + ((byte & 0x04) >> 2) + ((byte & 0x08) >> 3)
                    + ((byte & 0x10) >> 4) + ((byte & 0x20) >> 5)
                    + ((byte & 0x40) >> 6) + ((byte & 0x80) >> 7));
        });
}

namespace {
math::Extents2i extents(const boost::optional<imgproc::Crop2> &refRoi
                        , const math::Size2 &size, double sx, double sy)
{
    if (!refRoi) {
        // full
        return { 0, 0
                , int(std::ceil(size.width / sx))
                , int(std::ceil(size.height / sy)) };
    }

    math::Extents2i e(refRoi->x, refRoi->y, (refRoi->x + refRoi->width)
                      , (refRoi->y + refRoi->height));

    // limit to size
    if (e.ll(0) < 0) { e.ll(0) = 0; }
    if (e.ll(1) < 0) { e.ll(1) = 0; }
    if (e.ur(0) > size.width) { e.ur(0) = size.width; }
    if (e.ur(1) > size.height) { e.ur(1) = size.height; }

    // scale to mathc mask size
    return { int(std::floor(e.ll(0) / sx)), int(std::floor(e.ll(1) / sy))
            , int(std::ceil(e.ur(0) / sx)), int(std::ceil(e.ur(1) / sy)) };
}

} // namespace

int radius(const RasterMask &m
           , const boost::optional<math::Size2> &refSize
           , const boost::optional<imgproc::Crop2> &refRoi)
{
    auto size(m.dims());
    auto rs(refSize ? *refSize : size);

    double sx(double(rs.width) / size.width);
    double sy(double(rs.height) / size.height);

    // calculate roi extents
    auto roi(extents(refRoi, rs, sx, sy));

    // find out center of roi
    double cx((roi.ll(0) + roi.ur(0)) / 2.);
    double cy((roi.ll(1) + roi.ur(1)) / 2.);

    // radius^2
    double r2(0);

    for (int j(roi.ll(1)); j < roi.ur(1); ++j) {
        for (int i(roi.ll(0)); i < roi.ur(0); ++i) {
            if (m.get(i, j)) {
                // white pixel -> calculate radius^2
                auto nr2(math::sqr((i - cx) * sx)
                         + math::sqr((j - cy) * sy));
                if (nr2 > r2) {
                    // larger value
                    r2 = nr2;
                }
            }
        }
    }

    // return radius (as integer)
    return int(std::sqrt(r2));
}

void RasterMask::writeData(std::ostream &f) const
{
    write(f, mask_.get(), bytes_);
}

void RasterMask::readData(std::istream &f)
{
    read(f, mask_.get(), bytes_);
}

} } // namespace imgproc::bitfield
