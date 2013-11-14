#ifndef imgproc_tiff_hpp_included_
#define imgproc_tiff_hpp_included_

#include <memory>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <cstdint>

#include <boost/filesystem/path.hpp>

namespace imgproc { namespace tiff {

struct Error : std::runtime_error {
    Error(const std::string &msg) : std::runtime_error(msg) {}
};

typedef std::uint32_t Dir;

class Tiff {
public:
    friend Tiff openRead(const boost::filesystem::path &file);
    friend Tiff openWrite(const boost::filesystem::path &file);
    friend Tiff openAppend(const boost::filesystem::path &file);

    Dir currentDirectory();
    void setDirectory(Dir dirnum);
    Dir readDirectory();
    void writeDirectory();

private:
    typedef std::shared_ptr<void> Handle;
    Tiff(const Handle &handle);

    Handle handle_;
};

Tiff openRead(const boost::filesystem::path &file);
Tiff openWrite(const boost::filesystem::path &file);
Tiff openAppend(const boost::filesystem::path &file);

} } // namespace imgproc::tiff

#endif // imgproc_tiff_hpp_included_
