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

    Dir currentDirectory();
    void setDirectory(Dir dirnum);
    Dir readDirectory();
    void writeDirectory();
    bool isLastDirectory();

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
