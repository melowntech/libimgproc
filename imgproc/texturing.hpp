#ifndef imgproc_texturing_hpp_included_
#define imgproc_texturing_hpp_included_

#include <vector>

#include "math/geometry_core.hpp"

namespace imgproc { namespace tx {

/** Uv patch.
 */
struct UvPatch : math::Extents2 {
    UvPatch() : math::Extents2(math::InvalidExtents{}) {}

    void inflate(double size);
    void update(const UvPatch &other);
    void update(double x, double y);
};

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
     */
    Patch(const UvPatch &uvPatch);

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

    /** Whole pixel rectangle circumscribed around subpixel patch.
     */
    struct Rect {
        math::Point2i point;
        math::Size2i size;

        Rect() = default;
        Rect(const Rect&) = default;
        Rect(const UvPatch &uvPatch);
    };

    const Rect& src() const { return src_; }

    const Rect& dst() const { return dst_; }

    const math::Size2& size() const { return src_.size; }

    int width() const { return src_.size.width; }
    int height() const { return src_.size.height; }

private:
    /** Source patch origin (subpixel). Input value.
     */
    math::Point2 origin_;

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
math::Size2 pack(const Patch::list &patches);

// inlines

// TODO: make right for bilinear interpolation.
inline Patch::Rect::Rect(const UvPatch &uvPatch)
    : point(std::round(uvPatch.ll(0)), std::round(uvPatch.ll(1)))
    , size(std::round(uvPatch.ur(0)) - point(0) + 1
           , std::round(uvPatch.ur(1)) - point(1) + 1)
{}

inline Patch::Patch(const UvPatch &uvPatch)
    : origin_(uvPatch.ll)
    , src_(uvPatch), dst_(src_)
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

inline void UvPatch::inflate(double size)
{
    auto &self(static_cast<math::Extents2&>(*this));
    self = self + size;
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

} } // namespace imgproc::tx

#endif // imgproc_texturing_hpp_included_
