/**
 * @file rastermask.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (bitmap).
 */

#ifndef imgproc_restermask_hpp_included_
#define imgproc_restermask_hpp_included_

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iosfwd>

#include <boost/scoped_array.hpp>

#include <math/geometry_core.hpp>

namespace imgproc {

class RasterMask {
public:
    enum InitMode { EMPTY = 0, FULL = 1, SOURCE = 2 };

    RasterMask(const math::Size2 &size, InitMode mode);

    RasterMask(std::size_t width = 1, std::size_t height = 1
               , InitMode mode = EMPTY);

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask(const RasterMask &o, InitMode mode = SOURCE);

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask& operator=(const RasterMask &o);

    RasterMask& create(const math::Size2 &size, InitMode mode);

    RasterMask& create(std::size_t width, std::size_t height, InitMode mode) {
        return create(math::Size2(width, height), mode);
    }

    /** return size of mask */
    std::size_t size() const { return count_; }

    /** return total number of pixels */
    std::size_t capacity() const { return size_.width * size_.height; }

    /** test mask for emptiness */
    bool empty() const { return count_; }

    /** invert a mask (negate pixels) */
    void invert() {
        std::transform(mask_.get(), mask_.get() + bytes_
                       , mask_.get(), [](std::uint8_t b) { return ~b; });
    }

    /** FIXME: IMPLEMENT ME
     *  do a set difference with two masks. */
    void subtract( const RasterMask & op );

    /** implement! obtain mask value at given pos. */
    bool get(int x, int y) const;

    /** set mask value at given pos. */
    void set(int x, int y, bool value = true);

    /** FIXME: IMPLEMENT ME
     *  test if a given pixel is a boundary pixel (neighboring unset
     *  pixel in mask */
    bool onBoundary( int x, int y ) const;

    /** dump mask to stream */
    void dump(std::ostream &f);

    /** load mask from stream */
    void load(std::istream &f);

    void resetTrail();

    const math::Size2& dims() const { return size_; }

private:
    math::Size2 size_;
    std::size_t bytes_;
    boost::scoped_array<std::uint8_t> mask_;
    std::size_t count_;
};

// Inline method implementation

inline RasterMask::RasterMask(const math::Size2 &size, InitMode mode)
    : size_(size), bytes_((size.height * size.width + 7) >> 3)
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : size.height * size.width)
{
    std::memset(mask_.get(), (mode == EMPTY) ? 0x00 : 0xff, bytes_);
    resetTrail();
}

inline RasterMask::RasterMask(std::size_t width, std::size_t height
                              , InitMode mode)
    : size_(width, height), bytes_((height * width + 7) >> 3)
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : height * width)
{
    std::memset(mask_.get(), (mode == EMPTY) ? 0x00 : 0xff, bytes_);
    resetTrail();
}

/** initialize a mask of the same order, optionally copying mask. */
inline RasterMask::RasterMask(const RasterMask &o, InitMode mode)
    : size_(o.size_), bytes_(o.bytes_)
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : o.size_.height * o.size_.width)
{
    if (mode == SOURCE) {
        // deep copy
        std::memcpy(mask_.get(), o.mask_.get(), bytes_);
        resetTrail();
        count_ = o.count_;
    }
}

/** initialize a mask of the same order, optionally copying mask. */
inline RasterMask& RasterMask::operator=(const RasterMask &o) {
    if (&o == this) {
        return *this;
    }

    size_ = o.size_;
    bytes_ = o.bytes_;
    mask_.reset(new std::uint8_t[bytes_]);

    // deep copy
    std::memcpy(mask_.get(), o.mask_.get(), bytes_);
    count_ = o.count_;
    return *this;
}

inline RasterMask& RasterMask::create(const math::Size2 &size, InitMode mode)
{
    size_ = size;
    bytes_ = (size.height * size.width + 7) >> 3;
    mask_.reset(new std::uint8_t[bytes_]);
    count_ = ((mode == EMPTY) ? 0 : size.height * size.width);
    std::memset(mask_.get(), (mode == EMPTY) ? 0x00 : 0xff, bytes_);
    resetTrail();
    return *this;
}

inline bool RasterMask::get(int x, int y) const
{
    if ((x < 0) || (y < 0) || ((unsigned int)(x) >= size_.width)
        || ((unsigned int)(y) >= size_.height))
    {
        return false;
    }

    const auto offset(size_.width * y + x);
    return mask_[offset >> 3] & (1 << (offset & 0x07));
}

inline void RasterMask::set(int x, int y, bool value)
{
    if ((x < 0) || (y < 0) || ((unsigned int)(x) >= size_.width)
        || ((unsigned int)(y) >= size_.height))
    {
        return;
    }

    const auto offset(size_.width * y + x);
    const auto mask(1 << (offset & 0x07));
    auto &byteValue(mask_[offset >> 3]);
    if (value) {
        if (!(byteValue & mask)) {
            byteValue |= mask;
            ++count_;
        }
    } else {
        if (byteValue & mask) {
            byteValue &= ~mask;
            --count_;
        }
    }
}

inline void RasterMask::resetTrail()
{
    mask_[bytes_ - 1] &=
        (std::uint8_t(0xffu) >> ((bytes_ << 3) - size_.height * size_.width));
}

} // namespace imgproc

#endif // imgproc_restermask_hpp_included_