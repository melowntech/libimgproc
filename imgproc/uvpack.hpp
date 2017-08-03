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
#ifndef imgproc_uvpack_hpp_included_
#define imgproc_uvpack_hpp_included_

#include <opencv2/core/core.hpp>

namespace imgproc {

typedef cv::Point2f UVCoord;

/// Represents a rectangle in the UV texture space.
///
struct UVRect
{
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
    int x() const { return floor(min.x); }
    int y() const { return floor(min.y); }
    int width() const { return ceil(max.x) - x() + 1; }
    int height() const { return ceil(max.y) - y() + 1; }

    /// Map UV from view UV space to atlas UV space
    template<typename UVCoordType>
    void adjustUV(UVCoordType& uv) const {
        uv.x += packX - x();
        uv.y += packY - y();
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
