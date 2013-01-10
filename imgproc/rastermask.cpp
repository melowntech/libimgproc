/**
 * @file rastermask.cpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (bitmap).
 */

#include <iostream>
#include <stdexcept>

#include "dbglog/dbglog.hpp"

#include "rastermask.hpp"

namespace imgproc {

namespace {
    const char RASTERMASK_IO_MAGIC[5] = { 'R', 'M', 'A', 'S', 'K' };

    inline void write(std::ostream &os, const char *v, size_t count) {
        os.write(v, count);
    }

    template <typename T>
    void write(std::ostream &os, const T *v, size_t count)
    {
        os.write(reinterpret_cast<const char*>(v), count * sizeof(T));
    }

    template<typename T, int size>
    void write(std::ostream &os, const T(&v)[size]) {
        os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
    }

    template <typename T>
    void write(std::ostream &os, const T &v)
    {
        os.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }

    inline void read(std::istream &os, char *v, size_t count) {
        os.read(v, count);
    }

    template <typename T>
    void read(std::istream &os, T *v, size_t count)
    {
        os.read(reinterpret_cast<char*>(v), count * sizeof(T));
    }

    template<typename T, int size>
    void read(std::istream &os, T(&v)[size]) {
        os.read(reinterpret_cast<char*>(v), size * sizeof(T));
    }

    template <typename T>
    void read(std::istream &os, T &v)
    {
        os.read(reinterpret_cast<char*>(&v), sizeof(T));
    }
}

void RasterMask::dump(std::ostream &f)
{
    write(f, RASTERMASK_IO_MAGIC); // 5 bytes
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

    if (std::memcmp(magic, RASTERMASK_IO_MAGIC, sizeof(RASTERMASK_IO_MAGIC))) {
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

} // namespace imgproc
