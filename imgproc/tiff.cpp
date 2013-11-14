#include <tiffio.h>

#include "dbglog/dbglog.hpp"

#include "tiff.hpp"

namespace imgproc { namespace tiff {

typedef std::shared_ptr<TIFF> TiffHandle;

TIFF* TH(const std::shared_ptr<void> &handle)
{
    return static_cast<TIFF*>(handle.get());
}

namespace {

TiffHandle openTiff(const boost::filesystem::path &file
                    , const char *mode
                    , const std::string &message)
{
    auto h(::TIFFOpen(file.string().c_str(), mode));
    if (!h) {
        LOGTHROW(err1, Error)
            << "Unable to open tiff file " << file << ' ' << message << '.';
    }

    return TiffHandle(h, [](::TIFF *h) { ::TIFFClose(h); });
}

} // namespace

Tiff openRead(const boost::filesystem::path &file)
{
    return Tiff(openTiff(file, "r", "in read mode"));
}

Tiff openWrite(const boost::filesystem::path &file)
{
    return Tiff(openTiff(file, "w", "in write mode"));
}

Tiff openAppend(const boost::filesystem::path &file)
{
    return Tiff(openTiff(file, "a", "in append mode"));
}

Tiff::Tiff(const Handle &handle)
    : handle_(handle)
{}

Dir Tiff::readDirectory()
{
    if (!::TIFFReadDirectory(TH(handle_))) {
        LOGTHROW(err1, Error)
            << "Unable to read directory.";
    }
    return currentDirectory();
}

void Tiff::writeDirectory()
{
    if (!::TIFFWriteDirectory(TH(handle_))) {
        LOGTHROW(err1, Error)
            << "Unable to write directory.";
    }
}

Dir Tiff::currentDirectory()
{
    return TIFFCurrentDirectory(TH(handle_));
}

void Tiff::setDirectory(std::uint32_t dirnum)
{
    if (!::TIFFSetDirectory(TH(handle_), dirnum)) {
        LOGTHROW(err1, Error)
            << "Unable to set directory <" << dirnum << ">.";
    }
}

} } // namespace imgproc::tiff
