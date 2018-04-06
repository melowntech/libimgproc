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
/**
 * @file rastermask.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (bitmap).
 */

#ifndef imgproc_rastermask_bitfield_hpp_included_
#define imgproc_rastermask_bitfield_hpp_included_

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iosfwd>

#include <boost/scoped_array.hpp>
#include <boost/optional.hpp>

#include "math/geometry_core.hpp"

#include "../crop.hpp"
#include "./bitfieldfwd.hpp"

/**** bit-field version of rastermask ****/

namespace imgproc { namespace bitfield {

class RasterMask {
public:
    enum InitMode { EMPTY = 0, FULL = 1, SOURCE = 2 };

    RasterMask(const math::Size2 &size, InitMode mode);

    RasterMask(int width = 1, int height = 1, InitMode mode = EMPTY);

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask(const RasterMask &o, InitMode mode = SOURCE);

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask& operator=(const RasterMask &o);

    RasterMask& create(const math::Size2 &size, InitMode mode);

    RasterMask& create(int width, int height, InitMode mode) {
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

    /** Optimized version of set(x, y, true);
     */
    void add(int x, int y);

    /** Optimized version of set(x, y, false);
     */
    void remove(int x, int y);

    /** FIXME: IMPLEMENT ME
     *  test if a given pixel is a boundary pixel (neighboring unset
     *  pixel in mask */
    bool onBoundary( int x, int y ) const;

    /** dump mask to stream */
    void dump(std::ostream &f) const;

    /** load mask from stream */
    void load(std::istream &f);

    void resetTrail();

    const math::Size2& dims() const { return size_; }

    std::size_t byteCount() const { return bytes_; }

    /** Write binary data representation to stream */
    void writeData(std::ostream &f) const;

    /** Read binary data representation from stream */
    void readData(std::istream &f);

    /** Byte count from size.
     */
    static std::size_t byteCount(const math::Size2 &size) {
        return (size.height * size.width + 7) >> 3;
    }

private:
    math::Size2 size_;
    std::size_t bytes_;
    boost::scoped_array<std::uint8_t> mask_;
    std::size_t count_;
};

/** Calculate radius of restart mask (having circle center is its center)
 * \param m raster mask to use
 * \param refSize size of reference image if mask is not 1:1 of orignal image
 * \param refRoi region of interest inside original image
 */
int radius(const RasterMask &m
           , const boost::optional<math::Size2> &refSize = boost::none
           , const boost::optional<imgproc::Crop2> &refRoi = boost::none);

/** Generates bitfield raster mask from constant raster.
 *  See ../const-raster.hpp for const raster interface.
 *
 *  If the Inverse template parameter is false:
 *      * mask is created empty
 *      * valid pixels are added to mask
 *  If the Inverse template parameter is true:
 *      * mask is created full
 *      * only invalid pixels are removed from mask
 *
 *  \param raster source raster
 *  \param threshold thresholding function: raster value -> bool
 *  \return generated mask
 */
template <typename ConstRaster, typename Threshold, bool Inverse = false>
RasterMask fromRaster(const ConstRaster &raster, const Threshold &threshold);

// Inline method implementation

inline RasterMask::RasterMask(const math::Size2 &size, InitMode mode)
    : size_(size), bytes_(byteCount(size_))
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : math::area(size_))
{
    std::memset(mask_.get(), (mode == EMPTY) ? 0x00 : 0xff, bytes_);
    resetTrail();
}

inline RasterMask::RasterMask(int width, int height
                              , InitMode mode)
    : size_(width, height), bytes_(byteCount(size_))
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : math::area(size_))
{
    std::memset(mask_.get(), (mode == EMPTY) ? 0x00 : 0xff, bytes_);
    resetTrail();
}

/** initialize a mask of the same order, optionally copying mask. */
inline RasterMask::RasterMask(const RasterMask &o, InitMode mode)
    : size_(o.size_), bytes_(o.bytes_)
    , mask_(new std::uint8_t[bytes_])
    , count_((mode == EMPTY) ? 0 : math::area(size_))
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
    if ((x < 0) || (y < 0) || (x >= size_.width) || (y >= size_.height)) {
        return false;
    }

    const auto offset(size_.width * y + x);
    return mask_[offset >> 3] & (1 << (offset & 0x07));
}

inline void RasterMask::set(int x, int y, bool value)
{
    if ((x < 0) || (y < 0) || (x >= size_.width) || (y >= size_.height)) {
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

inline void RasterMask::add(int x, int y)
{
    if ((x < 0) || (y < 0) || (x >= size_.width) || (y >= size_.height)) {
        return;
    }

    const auto offset(size_.width * y + x);
    const auto mask(1 << (offset & 0x07));
    auto &byteValue(mask_[offset >> 3]);

    if (!(byteValue & mask)) {
        byteValue |= mask;
        ++count_;
    }
}

inline void RasterMask::remove(int x, int y)
{
    if ((x < 0) || (y < 0) || (x >= size_.width) || (y >= size_.height)) {
        return;
    }

    const auto offset(size_.width * y + x);
    const auto mask(1 << (offset & 0x07));
    auto &byteValue(mask_[offset >> 3]);

    if (byteValue & mask) {
        byteValue &= ~mask;
        --count_;
    }
}

inline void RasterMask::resetTrail()
{
    mask_[bytes_ - 1] &=
        (std::uint8_t(0xffu) >> ((bytes_ << 3) - size_.height * size_.width));
}

namespace detail {

template <bool Inverse>
struct MaskBuilder {
    MaskBuilder(RasterMask &mask) : mask(mask) {}
    RasterMask &mask;

    void operator()(int x, int y, bool value);
};

template<>
inline void MaskBuilder<false>::operator()(int x, int y, bool value)
{
    if (value) { mask.add(x, y); }
}

template<>
inline void MaskBuilder<true>::operator()(int x, int y, bool value)
{
    if (!value) { mask.remove(x, y); }
}

} // namespace detail

template <typename ConstRaster, typename Threshold, bool Inverse>
RasterMask fromRaster(const ConstRaster &raster, const Threshold &threshold)
{
    RasterMask mask(raster.size()
                    , (Inverse ? RasterMask::FULL : RasterMask::EMPTY));

    detail::MaskBuilder<Inverse> store(mask);

    for (int j(0), je(raster.height()); j != je; ++j) {
        for (int i(0), ie(raster.width()); i != ie; ++i) {
            store(i, j, threshold(raster(i, j)));
        }
    }

    return mask;
}

} } // namespace imgproc::bitfield

#endif // imgproc_rastermask_bitfield_hpp_included_
