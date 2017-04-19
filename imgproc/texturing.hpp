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
#ifndef imgproc_texturing_hpp_included_
#define imgproc_texturing_hpp_included_

#include <vector>

#include "math/geometry_core.hpp"

namespace imgproc { namespace tx {

/** Uv patch.
 */
struct UvPatch : math::Extents2 {
    UvPatch() : math::Extents2(math::InvalidExtents{}) {}
    UvPatch(const math::Extents2 &e) : math::Extents2(e) {}

    void inflate(double size);
    void update(const UvPatch &other);
    void update(double x, double y);
};

UvPatch inflate(const UvPatch &uvPatch, double size);

/** Texture patch. Maps source region to destination region.
 */
class Patch {
public:
    struct InplaceTag {};
    static InplaceTag Inplace;

    typedef std::vector<Patch*> list;

    /** Construct default, unusable patch.
     */
    Patch() = default;

    /** Creates patch for given texturing coordinates bounds.
     *
     * \param uvPatch source UV patch
     */
    Patch(const UvPatch &uvPatch);

    /** Manual patch creation.
     */
    Patch(int x, int y, int width, int height);

    /** Places patch at given location.
     */
    void place(const math::Point2i &location);

    /** Maps source texturing coordinates to destination texturing coordinates.
     */
    math::Point2d map(const math::Point2d &uv) const;

    /** Maps source texturing coordinates to destination texturing coordinates.
     *
     *  In place version.
     */
    void map(const InplaceTag&, math::Point2d &uv) const;

    /** Maps source texturing coordinates to destination texturing coordinates.
     *  Modifies arguments in place.
     */
    template <typename T>
    void map(T &x, T &y) const;

    math::Point2d imap(const math::Point2d &uv) const;

    /** Maps destination texturing coordinates to source texturing coordinates.
     *
     *  In place version.
     */
    void imap(const InplaceTag&, math::Point2d &uv) const;

    /** Maps destination texturing coordinates to source texturing coordinates.
     *  Modifies arguments in place.
     */
    template <typename T>
    void imap(T &x, T &y) const;

    /** Whole pixel rectangle circumscribed around subpixel patch.
     */
    struct Rect {
        math::Point2i point;
        math::Size2i size;

        Rect() = default;
        Rect(const Rect&) = default;
        Rect(const UvPatch &uvPatch);

        /** Manual rectangle creation.
         */
        Rect(int x, int y, int width, int height);
    };

    const Rect& src() const { return src_; }

    const Rect& dst() const { return dst_; }

    const math::Size2& size() const { return src_.size; }

    int width() const { return src_.size.width; }
    int height() const { return src_.size.height; }

    /** Clips source rectangle to given limits and updates destination one
     *  accordingly.
     */
    Patch& srcClip(const math::Size2 &limits);

    /** Clips source rectangle to given limits and updates destination one
     *  accordingly.
     */
    Patch& srcClip(int width, int height);

    /** Returns patch for source rectangle clipped to given limits.
     */
    Patch srcClipped(const math::Size2 &limits) const;

    /** Returns patch for source rectangle clipped to given limits.
     */
    Patch srcClipped(int width, int height) const;

private:
    /** Source rectangle.
     */
    Rect src_;

    /** Destination rectangle.
     */
    Rect dst_;

    /** Mapping between source and destination texturing coordinates.
     */
    math::Point2 shift_;
};

/** Packs texture patches.
 *  Returns size of resulting texture.
 */
math::Size2 pack(Patch::list &patches);

/** Packs texture patches.
 *  Returns size of resulting texture.
 *
 * Const vector interface.
 */
math::Size2 pack(const Patch::list &patches);

/** Generate container.
 *  Function Patch* asPatch(*iterator) must exist.
 */
template <typename Iterator>
math::Size2 pack(Iterator begin, Iterator end);

/** Default implementaion of asPatch for, well, patch itself.
 */
Patch* asPatch(Patch &patch);

// inlines

/**
 * To cover all source pixels for bilinear interpolation we have to have all 4
 * pixels aroud extreme patch edges, therefore we have to fix the extens as
 * follows:
 *
 * ll' = floor(ll - half pixel)
 * ur' = ceil(ur + half pixel)
 *
 * +1 is to get count of pixels between ll' (inclusive) and ur' (inclusive).
 */
inline Patch::Rect::Rect(const UvPatch &uvPatch)
    : point(std::floor(uvPatch.ll(0) - 0.5)
            , std::floor(uvPatch.ll(1) - 0.5))
    , size(std::ceil(uvPatch.ur(0) + 0.5) - point(0) + 1
           , std::ceil(uvPatch.ur(1) + 0.5) - point(1) + 1)
{}

inline Patch::Rect::Rect(int x, int y, int width, int height)
    : point(x, y), size(width, height)
{}

inline Patch::Patch(const UvPatch &uvPatch)
    : src_(uvPatch), dst_(src_)
{}

inline Patch::Patch(int x, int y, int width, int height)
    : src_(x, y, width, height), dst_(src_)
{}

inline void Patch::place(const math::Point2i &location)
{
    dst_.point = location;
    shift_(0) = dst_.point(0) - src_.point(0);
    shift_(1) = dst_.point(1) - src_.point(1);
}

inline math::Point2d Patch::map(const math::Point2d &uv) const
{
    return { uv(0) + shift_(0), uv(1) + shift_(1) };
}

inline void Patch::map(const InplaceTag&, math::Point2d &uv) const
{
    uv(0) += shift_(0);
    uv(1) += shift_(1);
}

template <typename T>
void Patch::map(T &x, T &y) const
{
    x += shift_(0);
    y += shift_(1);
}

inline math::Point2d Patch::imap(const math::Point2d &uv) const
{
    return { uv(0) - shift_(0), uv(1) - shift_(1) };
}

inline void Patch::imap(const InplaceTag&, math::Point2d &uv) const
{
    uv(0) -= shift_(0);
    uv(1) -= shift_(1);
}

template <typename T>
void Patch::imap(T &x, T &y) const
{
    x -= shift_(0);
    y -= shift_(1);
}

inline void UvPatch::inflate(double size)
{
    auto &self(static_cast<math::Extents2&>(*this));
    self = self + size;
}

inline UvPatch inflate(const UvPatch &uvPatch, double size)
{
    return static_cast<const math::Extents2&>(uvPatch) + size;
}

inline void UvPatch::update(double x, double y)
{
    math::update(*this, double(x), double(y));
}

inline void UvPatch::update(const UvPatch &other)
{
    math::update(*this, other.ll);
    math::update(*this, other.ur);
}

inline math::Size2 pack(const Patch::list &patches) {
    auto copy(patches);
    return pack(copy);
}

template <typename Iterator>
math::Size2 pack(Iterator begin, Iterator end)
{
    Patch::list patches;
    for (; begin != end; ++begin) {
        patches.push_back(asPatch(*begin));
    }
    return pack(patches);
}

inline Patch* asPatch(Patch &patch) { return &patch; }

inline Patch& Patch::srcClip(int width, int height)
{
    auto &sp(src_.point);
    auto &ss(src_.size);

    auto &dp(dst_.point);
    auto &ds(dst_.size);

    // clip origin to zero, update destination accordingly
    if (sp(0) < 0) {
        ss.width += sp(0);
        ds.width += sp(0);
        dp(0) -= sp(0);
        sp(0) = 0;
    }

    if (sp(1) < 0) {
        ss.height += sp(1);
        ds.height += sp(1);
        dp(1) -= sp(1);
        sp(1) = 0;
    }

    // clip length

    // compute overflow in x and y
    const auto xo(sp(0) + ss.width - width);
    const auto yo(sp(1) + ss.height - height);

    // and apply to width
    if (xo > 0) {
        ss.width -= xo;
        ds.width -= xo;
    }

    if (yo > 0) {
        ss.height -= yo;
        ds.height -= yo;
    }

    return *this;
}

inline Patch& Patch::srcClip(const math::Size2 &limits)
{
    return srcClip(limits.width, limits.height);
}

inline Patch Patch::srcClipped(int width, int height) const
{
    return Patch(*this).srcClip(width, height);
}

inline Patch Patch::srcClipped(const math::Size2 &limits) const
{
    return srcClipped(limits.width, limits.height);
}

} } // namespace imgproc::tx

#endif // imgproc_texturing_hpp_included_
