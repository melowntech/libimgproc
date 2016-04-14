#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/filesystem.hpp"
#include "utility/streams.hpp"
#include "utility/binaryio.hpp"
#include "utility/align.hpp"

#include "./mappedqtree.hpp"

namespace bi = boost::interprocess;
namespace bin = utility::binaryio;

namespace imgproc { namespace mappedqtree {

namespace {

const char IO_MAGIC[6] = { 'M', 'Q', 'M', 'A', 'S', 'K' };

} // namespcace

struct MemoryBase {
    MemoryBase(const boost::filesystem::path &path, std::size_t offset)
        : treeStart(), size(), depth()
    {
        utility::ifstreambuf f(path.string());
        f.seekg(offset);

        char magic[6];
        bin::read(f, magic);
        if (std::memcmp(magic, IO_MAGIC, sizeof(IO_MAGIC))) {
            LOGTHROW(err2, std::runtime_error)
                << "Mapped QTree RasterMask has wrong magic.";
        }

        uint8_t reserved;
        bin::read(f, reserved); // reserved
        bin::read(f, reserved); // reserved

        uint8_t tmpDepth;
        bin::read(f, tmpDepth);
        depth = tmpDepth;

        // read size
        std::uint32_t tmpSize;
        bin::read(f, tmpSize);

        // here start data
        treeStart = f.tellg();
        size = tmpSize + treeStart;
    }

    char *data;
    std::size_t treeStart;
    std::size_t size;
    unsigned int depth;
};

struct RasterMask::Memory : MemoryBase {
    Memory(const boost::filesystem::path &path, std::size_t offset)
        : MemoryBase(path, offset)
        , file(path.c_str(), bi::read_only)
        , region(file, bi::read_only, 0, treeStart + size)
        , data()
    {

        data = static_cast<char*>(region.get_address());
    }

    bi::file_mapping file;
    bi::mapped_region region;
    const char *data;
};

RasterMask::RasterMask()
    : memory_(), data_(), dataSize_(), depth_(), start_()
{}

RasterMask::RasterMask(const boost::filesystem::path &path
                       , std::size_t offset)
    : memory_(std::make_shared<Memory>(path, offset))
    , data_(memory_->data), dataSize_(memory_->size)
    , depth_(memory_->depth)
    , start_(memory_->treeStart)
{
}

} } // namespace imgproc::mappedqtree
