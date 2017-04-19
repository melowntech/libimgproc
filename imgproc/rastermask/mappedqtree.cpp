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
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/filesystem.hpp"
#include "utility/streams.hpp"
#include "utility/binaryio.hpp"
#include "utility/align.hpp"

#include "./mappedqtree.hpp"
#include "./quadtree.hpp"

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

RasterMask::RasterMask(const boost::optional<boost::filesystem::path> &path
                       , std::size_t offset)
    : memory_(path ? std::make_shared<Memory>(*path, offset)
              : nullptr)
    , data_(memory_ ? memory_->data :nullptr)
    , dataSize_(memory_ ? memory_->size : 0)
    , depth_(memory_ ? memory_->depth : 0)
    , start_(memory_ ? memory_->treeStart : 0)
{
}

void RasterMask::write(std::ostream &f, const quadtree::RasterMask &mask
                       , unsigned int depth, unsigned int x, unsigned int y)
{
    typedef quadtree::RasterMask::Node Node;
    typedef quadtree::RasterMask::NodeType NodeType;

    struct Writer {
        Writer(const Node &node, std::ostream &f)
            : f(f)
        {
            write(node);
        }

        inline std::uint8_t bitValue(NodeType type, std::uint8_t offset) {
            switch (type) {
            case NodeType::WHITE: return 0x3 << (2 * offset);
            case NodeType::BLACK: return 0x0;
            case NodeType::GRAY: return 0x1 << (2 * offset);
            }
            return 0;
        }

        inline void writeSubtree(const Node &node) {
            if (node.type != NodeType::GRAY) { return; }

            // record current position and align it to sizeof jump value
            auto jump(utility::align(f.tellp(), sizeof(std::uint32_t)));

            // allocate space for jump offset
            f.seekp(jump + std::streampos(sizeof(std::uint32_t)));

            // write subtree
            write(node);

            // remember current position
            auto end(f.tellp());

            // rewind to allocated space
            f.seekp(jump);

            // calculate jump value (take size of jump value into account)
            std::uint32_t jumpValue(end - jump - sizeof(jumpValue));
            bin::write(f, jumpValue);

            // jump to the end again
            f.seekp(end);
        }

        inline void write(const Node &node) {
            std::uint8_t value
                (bitValue(node.children->ul.type, 3)
                 | bitValue(node.children->ur.type, 2)
                 | bitValue(node.children->ll.type, 1)
                 | bitValue(node.children->lr.type, 0));

            bin::write(f, value);

            writeSubtree(node.children->ul);
            writeSubtree(node.children->ur);
            writeSubtree(node.children->ll);
            writeSubtree(node.children->lr);
        }

        std::ostream &f;
    };

    bin::write(f, IO_MAGIC); // 6 bytes
    bin::write(f, uint8_t(0)); // reserved
    bin::write(f, uint8_t(0)); // reserved

    // write depth
    auto d(mask.depth() - depth);
    bin::write(f, std::uint8_t(d));

    // make room for data size
    auto sizePlace(f.tellp());
    f.seekp(sizePlace + std::streampos(sizeof(std::uint32_t)));

    if (const auto *start = mask.findSubtree(depth, x, y)) {
        // write root node or descend
        switch (start->type) {
        case NodeType::WHITE:
            bin::write(f, std::uint8_t(0xff));
            break;

        case NodeType::BLACK:
            bin::write(f, std::uint8_t(0x00));
            break;

        default:
            Writer(*start, f);
            break;
        }
    } else {
        // subtree not found -> black by definition
        bin::write(f, std::uint8_t(0x00));
    }

    // compute data size and write to pre-allocated place
    auto end(f.tellp());
    f.seekp(sizePlace);
    std::uint32_t size(end - sizePlace - sizeof(std::uint32_t));
    bin::write(f, size);

    // move back to the end
    f.seekp(end);
}

} } // namespace imgproc::mappedqtree
