#include <cstring>

#include <boost/thread.hpp>

#include <tiffio.h>

#include "dbglog/dbglog.hpp"

#include "bintiff.hpp"

extern "C" {

void imgproc_tiff_errorHandler(const char *module, const char *fmt
                               , va_list ap);

void imgproc_tiff_warningHandler(const char *module, const char *fmt
                                 , va_list ap);

} // extern "C"

namespace imgproc { namespace tiff {

namespace detail {

boost::thread_specific_ptr<std::string> lastError_;

void setLastError(const char *module, const char *message)
{
    lastError_.reset(new std::string(module));
    lastError_->append(message);
}

std::string getLastError()
{
    if (!lastError_.get()) { return {}; }
    auto e(*lastError_.get());
    lastError_.reset();
    return e;
}

bool errorOccured()
{
    return lastError_.get();
}

void resetError()
{
    lastError_.reset();
}

} // detail

namespace {

typedef std::shared_ptr<TIFF> TiffHandle;

TIFF* TH(const std::shared_ptr<void> &handle)
{
    return static_cast<TIFF*>(handle.get());
}

TIFF* TH(TIFF *handle)
{
    return handle;
}

TiffHandle openTiff(const boost::filesystem::path &file
                    , const char *mode
                    , const std::string &message)
{
    auto h(::TIFFOpen(file.string().c_str(), mode));
    if (!h) {
        LOGTHROW(err1, Error)
            << "Unable to open tiff file " << file << ' ' << message
            << " (" << detail::getLastError() << ").";
    }

    return TiffHandle(h, [](::TIFF *h) { ::TIFFClose(h); });
}

} // namespace

BinTiff openRead(const boost::filesystem::path &file)
{
    return BinTiff(openTiff(file, "r", "in read mode"));
}

BinTiff openWrite(const boost::filesystem::path &file)
{
    return BinTiff(openTiff(file, "w", "in write mode"));
}

BinTiff openAppend(const boost::filesystem::path &file)
{
    return BinTiff(openTiff(file, "a", "in append mode"));
}

BinTiff::BinTiff(const Handle &handle)
    : handle_(handle)
{}

namespace {
template <typename Handle, typename ...Args>
void setField(Handle &&handle, ttag_t tag, Args &&...args)
{
    if (!::TIFFSetField(TH(handle), tag, args...)) {
        LOGTHROW(err1, Error)
            << "Unable to set field <" << tag << "> ("
            << detail::getLastError() << ").";
    }
}

template <typename Handle, typename ...Args>
void getField(Handle &&handle, ttag_t tag, Args &&...args)
{
    if (!::TIFFGetField(TH(handle), tag, args...)) {
        LOGTHROW(err1, Error)
            << "Unable to get field <" << tag << "> ("
            << detail::getLastError() << ").";
    }
}

class Writer {
public:
    Writer(const std::shared_ptr<void> &handle
           , std::size_t blockSize)
        : h_(TH(handle)), blockSize_(blockSize), row_(0)
        , buf_(new char[blockSize])
        , begin_(buf_.get())
        , left_(blockSize_)
    {}

    template <typename T>
    void write(const T &value) {
        writeRaw(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    void write(const std::string &value) {
        writeRaw(value.data(), value.size());
    }

    void flush() {
        if (left_ != blockSize_) {
            if (left_) {
                // clear rest of buffer
                std::memset(begin_, 0, left_);
            }

            // something unwritten in the buffer

            LOG(info4) << "writing row " << row_;
            if (::TIFFWriteScanline(h_, buf_.get(), row_) != 1) {
                LOGTHROW(err1, Error)
                    << "Unable to write row " << row_ << " to tiff ("
                    << detail::getLastError() << ").";
            }

            begin_ = buf_.get();
            left_ = blockSize_;
            ++row_;
        }
    }

private:
    void writeRaw(const void *data_, std::size_t size) {
        const char *data(static_cast<const char*>(data_));
        while (size) { writeRawOne(data, size); }
    }

    void writeRawOne(const char *&data, std::size_t &size) {
        auto toWrite((size < left_) ? size : left_);
        std::memcpy(begin_, data, toWrite);
        begin_ += toWrite;
        data += toWrite;
        left_ -= toWrite;
        size -= toWrite;

        // flush buffer
        if (!left_) { flush(); }
    }

    TIFF *h_;
    std::size_t blockSize_;
    std::uint32_t row_;

    std::unique_ptr<char[]> buf_;
    char *begin_;
    std::size_t left_;
};

class Reader {
public:
    Reader(const std::shared_ptr<void> &handle)
        : h_(TH(handle)), blockSize_(::TIFFScanlineSize(h_)), row_(0)
        , buf_(new char[blockSize_]), begin_(buf_.get()), left_(0)
    {}

    template <typename T>
    void read(T &value) {
        readRaw(reinterpret_cast<char*>(&value), sizeof(T));
    }

    void read(std::string &value, std::size_t size) {
        while (size) { readRawOne(value, size); }
    }

private:
    void readRaw(void *data_, std::size_t size) {
        char *data(static_cast<char*>(data_));
        while (size) { readRawOne(data, size); }
    }

    void readRawOne(char *&data, std::size_t &size) {
        fetch();

        auto toRead((size < left_) ? size : left_);
        std::memcpy(data, begin_, toRead);
        begin_ += toRead;
        data += toRead;
        left_ -= toRead;
        size -= toRead;
    }

    void readRawOne(std::string &data, std::size_t &size) {
        fetch();

        auto toRead((size < left_) ? size : left_);
        data.append(begin_, toRead);
        begin_ += toRead;
        left_ -= toRead;
        size -= toRead;
    }

    void fetch() {
        if (left_) { return; }

        LOG(info4) << "reading row " << row_;

        begin_ = buf_.get();
        if (::TIFFReadScanline(h_, begin_, row_) != 1) {
            LOGTHROW(err1, Error)
                << "Unable to read row " << row_ << " from tiff ("
                << detail::getLastError() << ").";
        }
        left_ = blockSize_;
        ++row_;
    }

    TIFF *h_;
    std::size_t blockSize_;
    std::uint32_t row_;

    std::unique_ptr<char[]> buf_;
    char *begin_;
    std::size_t left_;
};

} // namespace

void BinTiff::write(const std::string &data)
{
    constexpr std::size_t BLOCK_SIZE(1024);

    // setup fields
    setField(handle_, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    setField(handle_, TIFFTAG_BITSPERSAMPLE, 8);
    setField(handle_, TIFFTAG_SAMPLESPERPIXEL, 1);
    setField(handle_, TIFFTAG_IMAGEWIDTH, BLOCK_SIZE);
    setField(handle_, TIFFTAG_IMAGELENGTH, 1);

    Writer writer(handle_, BLOCK_SIZE);
    writer.write(std::uint32_t(data.size()));
    writer.write(data);
    writer.flush();

    // write directory
    if (!::TIFFRewriteDirectory(TH(handle_))) {
        LOGTHROW(err1, Error)
            << "Unable to write directory (" << detail::getLastError() << ").";
    }
}

std::string BinTiff::read()
{
    LOG(info4) << "reading";

    std::uint32_t dataSize;
    std::string out;

    Reader reader(handle_);
    reader.read(dataSize);
    reader.read(out, dataSize);

    return out;
}

namespace {

int findFile(TIFF *h, const std::string &filename)
{
    LOG(info4) << "Rewind";
    int dir(0);
    if (!::TIFFSetDirectory(h, 0)) {
        // no directory at all
        LOG(info4) << "No directory 0.";
        return -1;
    }

    LOG(info4) << "Scanning: " << ::TIFFCurrentDirectory(h);
    do {
        LOG(info4) << "Inside: " << ::TIFFCurrentDirectory(h);
        const char *fname;
        if (::TIFFGetField(h, TIFFTAG_DOCUMENTNAME, &fname)) {
            LOG(info4) << "Found filename: " << fname;
            if (filename == fname) { return dir; }
        }
        ++dir;
    } while (::TIFFReadDirectory(h));

    return -1;
}

} // namespace

void BinTiff::create(const std::string &filename)
{
    // seek to the file (if exists)
    auto dir(findFile(TH(handle_), filename));
    if (dir < 0) {
        // not found, create new record
        ::TIFFCreateDirectory(TH(handle_));
    }

    LOG(info4) << "Current dir: " << ::TIFFCurrentDirectory(TH(handle_));
    setField(handle_, TIFFTAG_DOCUMENTNAME, filename.c_str());
}

void BinTiff::seek(const std::string &filename)
{
    if (findFile(TH(handle_), filename) < 0) {
        LOGTHROW(err1, Error)
            << "No such file " << filename << " in tiff.";
    }
}

namespace detail {

struct Init {
    Init() {
        ::TIFFSetErrorHandler(&::imgproc_tiff_errorHandler);
        ::TIFFSetWarningHandler(&::imgproc_tiff_warningHandler);
    }
};

Init init_;

} // namespace detail

} } // namespace imgproc::tiff

extern "C" {

void imgproc_tiff_errorHandler(const char *module, const char *fmt
                               , va_list ap)
{
    char buf[1024];
    ::vsnprintf(buf, sizeof(buf), fmt, ap);
    buf[sizeof(buf) - 1] = '\0';
    imgproc::tiff::detail::setLastError(module, buf);
    LOG(err1) << "libtiff: " << module << buf;
}

void imgproc_tiff_warningHandler(const char *module, const char *fmt
                                 , va_list ap)
{
    char buf[1024];
    ::vsnprintf(buf, sizeof(buf), fmt, ap);
    buf[sizeof(buf) - 1] = '\0';
    LOG(warn1) << "libtiff: " << module << buf;
}

} // extern "C"
