/**
 * @file rastermask/mappedqtree.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Memory-mapped quad-tree raster mask
 */

#ifndef imgproc_rastermask_mappedqtree_hpp_included_
#define imgproc_rastermask_mappedqtree_hpp_included_

#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iosfwd>

#include <boost/filesystem/path.hpp>
#include <boost/logic/tribool.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/align.hpp"

#include "math/geometry_core.hpp"

/**** mappedqtree version of rastermask ****/

namespace imgproc { namespace mappedqtree {

class RasterMask {
public:
    RasterMask();

    RasterMask(const boost::filesystem::path &path
               , std::size_t offset = 0);

    RasterMask(const RasterMask&) = default;
    RasterMask& operator=(const RasterMask&) = default;

    typedef math::Extents2_<unsigned int> Extents;

    struct Constraints {
        unsigned int depth;
        Extents extents;

        Constraints(unsigned int depth = ~0)
            : depth(depth)
            , extents(math::InvalidExtents{})
        {}
    };

    template <typename Op>
    void forEachQuad(const Op &op, const Constraints &constraints
                     = Constraints()) const;

    unsigned int depth() const { return depth_; }

    math::Size2i size() const {
        return math::Size2i(1 << depth_, 1 << depth_);
    }

private:
    template <typename T>
    const T& read(std::size_t &index) const;

    template <typename Op>
    void forEachQuad(const Op &op, unsigned int depthLimit
                     , const Extents *extents)
        const;

    /** Called from RasterMask::forEachQuad */
    template <typename Op>
    void descend(unsigned int x, unsigned int y, unsigned int size
                 , std::size_t index, const Op &op
                 , unsigned int depthLimit, const Extents *extents) const;

    struct Memory;
    std::shared_ptr<Memory> memory_;

    // data managed inside a Memory object
    const char *data_;
    std::size_t dataSize_;

    unsigned int depth_;
    std::size_t start_;
};

struct AsNode { std::uint8_t value; };

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const AsNode &node)
{
    auto type([&](std::uint8_t offset) -> std::uint8_t
    {
        return ((node.value >> (2 * offset)) & 0x3);
    });

    for (auto o : { 3, 2, 1, 0 }) {
        switch (type(o)) {
        case 0x0: os << 'B'; break;
        case 0x3: os << 'W'; break;
        default: os << 'G';
        }
    }
    return os;
}

template <typename T>
const T& RasterMask::read(std::size_t &index) const
{
    index = utility::align(index, sizeof(T));
    const auto &value(*reinterpret_cast<const T*>(data_ + index));
    index += sizeof(T);
    return value;
}

template <typename Op>
void RasterMask::forEachQuad(const Op &op, const Constraints &constraints)
    const
{
    forEachQuad(op, (constraints.depth > depth_) ? depth_ : constraints.depth
                , (math::valid(constraints.extents)
                   ? &constraints.extents : nullptr));
}

namespace detail {

inline bool checkExtents(const RasterMask::Extents *&extents, unsigned int x
                         , unsigned int y, unsigned int size)
{
    if (!extents) { return true; }

    // TODO: implement me
    (void) x;
    (void) y;
    (void) size;
    return true;
}

} // detail

template <typename Op>
void RasterMask::forEachQuad(const Op &op, unsigned int depthLimit
                             , const Extents *extents) const
{
    // this is special handling for root node that has no explicit
    // representation
    std::size_t root(start_);
    // load node value
    const auto value(read<std::uint8_t>(root));
    auto size(1 << depth_);

    if (!detail::checkExtents(extents, 0, 0, size)) { return; }

    // handle value
    switch (value) {
    case 0x00:
        // root is black leaf node
        op(0, 0, size, false);
        return;

    case 0xff:
        // root is white leaf node
        op(0, 0, size, true);
        return;

    default: break;
    }

    // root has some children: handle if at allowed bottom
    if (!depthLimit) {
        // bottom -> execute
        op(0, 0, size, boost::indeterminate);
        return;
    }

    // descend down the tree
    descend(0, 0, size, start_, op, depthLimit, extents);
}

template <typename Op>
void RasterMask::descend(unsigned int x, unsigned int y
                         , unsigned int size, std::size_t index
                         , const Op &op
                         , unsigned int depthLimit, const Extents *extents)
    const
{
    // get children value
    const auto children(read<std::uint8_t>(index));

    auto type([&](std::uint8_t offset) -> std::uint8_t
    {
        return ((children >> (2 * offset)) & 0x3);
    });

    auto processSubtree([&](std::uint8_t type
                            , unsigned int depthLimit
                            , unsigned int x, unsigned int y
                            , unsigned int size) -> void
    {
        if (!detail::checkExtents(extents, 0, 0, size)) { return; }

        switch (type) {
        case 0x0:
            // black node
            op(x, y, size, false);
            return;

        case 0x3:
            // white node
            op(x, y, size, true);
            return;

        default: break; // gray
        }

        // jump over this gray node
        auto jump(read<std::uint32_t>(index));

        if (!depthLimit) {
            // bottom -> execute
            op(x, y, size, boost::indeterminate);
            index += jump;
            return;
        }

        // descend
        descend(x, y, size, index, op, depthLimit, extents);
        index += jump;
    });

    const auto split(size / 2);
    processSubtree(type(3), depthLimit - 1, x, y, split);
    processSubtree(type(2), depthLimit - 1, x + split, y, split);
    processSubtree(type(1), depthLimit - 1, x, y + split, split);
    processSubtree(type(0), depthLimit - 1, x + split, y + split, split);
}

} } // namespace imgproc::mappedqtree

#endif // imgproc_rastermask_mappedqtree_hpp_included_
