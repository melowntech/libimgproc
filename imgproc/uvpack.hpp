#ifndef imgproc_uvpack_hpp_included_
#define imgproc_uvpack_hpp_included_

#include <opencv2/core/core.hpp>

namespace imgproc {

typedef cv::Point2f UVCoord;

/// Represents a rectangle in the UV texture space.
///
struct UVRect
{
    struct PixelTag {};
    static PixelTag Pixel;

    UVCoord min, max; ///< min-max corners in the source texture
    int packX, packY; ///< position in the packed texture

    UVRect() { clear(); }

    /// Inflate the rectangle to include the given point.
    void update(UVCoord uv);

    /// Initialize for update().
    void clear() {
        min.x = min.y = INFINITY;
        max.x = max.y = -INFINITY;
        packX = packY = 0;
    }

    /// Returns true if the rectangle has been initialized
    bool valid() const { return min.x <= max.x; }

    /// Merge with another rect and return true, or do nothing and return false
    /// in case the area of the result would be larger than the sum of the areas
    /// of the two original rectangles.
    bool merge(UVRect& other);

    /// Grow the rectangle by the given margin.
    void inflate(double margin) {
        min.x -= margin;  min.y -= margin;
        max.x += margin;  max.y += margin;
    }

    /// Return exact area
    double area() const {
        return (max.x - min.x) * (max.y - min.y);
    }

    // Dimensions in whole pixels
    int x() const { return floor(min.x - 0.5f); }
    int y() const { return floor(min.y - 0.5f); }
    int width() const { return ceil(max.x + 0.5f) - x(); }
    int height() const { return ceil(max.y + 0.5f) - y(); }

    /// Map UV from view UV space to atlas UV space
    template<typename UVCoordType>
    void adjustUV(UVCoordType& uv) const {
        uv.x += packX - x();
        uv.y += packY - y();
    }

    // Dimensions in pixel coordinates
    int x(const PixelTag&) const { return std::round(min.x); }
    int y(const PixelTag&) const { return std::round(min.y); }

    int width(const PixelTag&) const {
        return std::round(max.x) - std::round(min.x) + 1;
    }
    int height(const PixelTag&) const {
        return std::round(max.y) - std::round(min.y) + 1;
    }

    /** Grab source rectangle (from (x, y) to (x + width, y + height)
     */
    cv::Rect srcRect(const PixelTag&) const {
        const auto xs(std::round(min.x));
        const auto ys(std::round(min.y));
        return cv::Rect(xs, ys
                        , std::round(max.x) - xs + 1
                        , std::round(max.y) - ys + 1);
    }

    /** Grab destination rectangle (from (packX, packY)
     *  to (x + width, y + height)
     */
    cv::Rect dstRect(const PixelTag &tag) const {
        return cv::Rect(packX, packY, width(tag), height(tag));
    }

    /// Map UV from view UV space to atlas UV space
    template<typename UVCoordType>
    void adjustUV(UVCoordType& uv, const PixelTag &tag) const {
        uv.x += packX - x(tag);
        uv.y += packY - y(tag);
    }

    /// Return true if the given rectangle lies inside this rectangle.
    bool contains(const UVRect& rect) const {
        return rect.min.x >= min.x && rect.max.x <= max.x &&
               rect.min.y >= min.y && rect.max.y <= max.y;
    }
};


/// Calculates a (not necessarily optimal) packing of small rectangles into one
/// big rectangle (texture). The class maintains a list of free areas (initially
/// the whole rectangle) and each new rectangle uses one of the free rectangles,
/// splitting the remaining space into two new parts and the process repeats.
/// The rectangles are collected first with addRect() and later sorted by size
/// and packed (starting with the largest) using the method pack(). If the rect-
/// angles don't fit, the total pack area is doubled repeatedly until they do.
///
class RectPacker
{
public:

    RectPacker() : packWidth(0), packHeight(0) {}

    /// Add a rectangle to the internal pack list.
    void addRect(UVRect* rect)
        { list.push_back(rect); }

    /// Pack the rectangles, updating their packX and packY
    void pack();

    int width() const { return packWidth; }
    int height() const { return packHeight; }

protected:

    struct Node
    {
        UVRect* rect; ///< nonzero means regular node, 0 means leaf (unallocated)
        int x, y, width, height; ///< this node position and size
        int remaining; ///< space remaining in this node and its subtree
        Node* son[2]; ///< son[0] is space below, son[1] is space to the right

        Node(int x, int y, int w, int h)
            : rect(0), x(x), y(y), width(w), height(h), remaining(w*h)
            { son[0] = son[1] = 0; }

        ~Node() {
            if (son[0]) delete son[0];
            if (son[1]) delete son[1];
        }

        Node* findSpace(int rw, int rh); ///< find leaf large enough
    };

    int packWidth, packHeight;
    std::vector<UVRect*> list;
};


} // namespace imgproc

#endif // imgproc_uvpack_hpp_included_
