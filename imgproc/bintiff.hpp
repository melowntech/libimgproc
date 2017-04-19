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
#ifndef imgproc_bintiff_hpp_included_
#define imgproc_bintiff_hpp_included_

#include <memory>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <cstdint>
#include <cstdio>

#include <tiff.h>

#include <boost/filesystem/path.hpp>

namespace imgproc { namespace tiff {

struct Error : std::runtime_error {
    Error(const std::string &msg) : std::runtime_error(msg) {}
};

struct NoSuchFile : Error {
    NoSuchFile(const std::string &msg) : Error(msg) {}
};

typedef std::uint32_t Dir;

class BinTiff;

BinTiff openRead(const boost::filesystem::path &file);
BinTiff openWrite(const boost::filesystem::path &file);
BinTiff openAppend(const boost::filesystem::path &file);

class BinTiff {
public:
    friend BinTiff openRead(const boost::filesystem::path &file);
    friend BinTiff openWrite(const boost::filesystem::path &file);
    friend BinTiff openAppend(const boost::filesystem::path &file);

    class OBinStream; friend class OBinStream;
    class IBinStream; friend class IBinStream;

    IBinStream istream(const std::string &filename);
    OBinStream ostream(const std::string &filename);

private:
    std::string read();
    void write(const std::string &data);
    void create(const std::string &filename);
    void seek(const std::string &filename);

    typedef std::shared_ptr<void> Handle;
    BinTiff(const Handle &handle);

    Handle handle_;
};

class BinTiff::OBinStream {
public:
    OBinStream(BinTiff *tiff, const std::string &filename)
        : tiff_(tiff), os_(new std::ostringstream)
    {
        tiff_->create(filename);
    }
    OBinStream(OBinStream &&) = default;

    ~OBinStream() {
        if (os_) {
            tiff_->write(os_->str());
        }
    }
    operator std::ostream&() { return *os_; };

private:
    BinTiff *tiff_;
    std::unique_ptr<std::ostringstream> os_;
};

class BinTiff::IBinStream {
public:
    IBinStream(BinTiff *tiff, const std::string &filename)
        : tiff_(tiff), os_()
    {
        tiff_->seek(filename);
        os_.reset(new std::istringstream(tiff_->read()));
    }
    IBinStream(IBinStream &&) = default;

    operator std::istream&() { return *os_; };

private:
    BinTiff *tiff_;
    std::unique_ptr<std::istringstream> os_;
};

inline BinTiff::IBinStream BinTiff::istream(const std::string &filename)
{
    return IBinStream(this, filename);
}

inline BinTiff::OBinStream BinTiff::ostream(const std::string &filename)
{
    return OBinStream(this, filename);
}

} } // namespace imgproc::tiff

#endif // imgproc_bintiff_hpp_included_
