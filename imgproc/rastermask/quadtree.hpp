/**
 * @file rastermask/quadtree.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Raster mask (bitmap).
 */

#ifndef imgproc_rastermask_quadtree_hpp_included_
#define imgproc_rastermask_quadtree_hpp_included_

#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iosfwd>

#include <boost/scoped_array.hpp>

#include "math/geometry_core.hpp"

#include "./bitfieldfwd.hpp"

/**** quad-tree version of rastermask ****/

namespace imgproc { namespace quadtree {

class RasterMask {
public :
    enum InitMode {
        EMPTY = 0,
        FULL = 1,
        SOURCE = 2
    };

    RasterMask() : sizeX_(0), sizeY_(0), count_(0), root_(*this) {}

    /** initialize mask */
    RasterMask( uint sizeX, uint sizeY, const InitMode mode );

    /** intialize mask */
    RasterMask( const math::Size2 & size, const InitMode mode );

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask( const RasterMask & mask, const InitMode mode = SOURCE );

    /** return size of mask */
    math::Size2 size() const { return math::Size2(sizeX_, sizeY_); }

    /** assignment operator */
    RasterMask & operator = ( const RasterMask & );

    /** destroy */
    ~RasterMask() {}

    /** invert a mask (negate pixels) */
    void invert();

    /** do a set difference with two masks. */
    void subtract(const RasterMask &op);

    /** obtain mask value at given pos, return false if x, y out of bounds */
    bool get( int x, int y ) const;

    /** obtain mask value at given pos, clamp x, y to nearest boundary position */
    bool getClamped( int x, int y ) const;

    /** set mask value at given pos. */
    void set( int x, int y, bool value = true );

    /** test if a given pixel is a boundary pixel (neighboring unset
        pixel in mask */
    bool onBoundary( int x, int y ) const;

    /** return mask size (number of white pixels) */
    ulong count() const { return count_; }

    /** return total number of pixels */
    ulong capacity() const { return ulong(sizeX_) * ulong(sizeY_); }

    /** test mask for emptiness */
    bool empty() const { return count_ == 0; }

    /** test mask for zero size */
    bool zeroSize() const { return !capacity(); }

    /** dump mask to stream */
    void dump( std::ostream & f ) const;

    /** load mask from stream */
    void load( std::istream & f );

    /** dump mask to bitfield mask */
    imgproc::bitfield::RasterMask asBitfield() const;

    math::Size2 dims() const { return math::Size2(sizeX_, sizeY_); }

    enum class Filter {
        black, white, both
    };

    /** Runs op(x, y, xsize, ysize, white) for each black/white quad.
     */
    template <typename Op>
    void forEachQuad(const Op &op, Filter filter = Filter::both) const;

    /** Runs op(x, y, white) for each black/white pixel.
     *  Uses forEachQuad and rasterizes quad internally.
     */
    template <typename Op>
    void forEach(const Op &op, Filter filter = Filter::both)  const;

    /** Merges other's quadtree in this quadtree.
     *
     * If checkDimensions is false you need to know what you are doing!
     */
    void merge(const RasterMask &other, bool checkDimensions = true);

    /** In place intersects other's quadtree with this quadtree.
     */
    void intersect(const RasterMask &other);

    /** Makes mask coarsen. White quads smaller than given threshold grow to
     *  match threshold.
     */
    void coarsen(const uint threshold = 2);

private :
    void recount();

    enum NodeType { WHITE, BLACK, GRAY };

    struct NodeChildren;

    struct Node
    {
        Node(RasterMask &mask)
            : type(BLACK), mask(mask), children() {};
        Node(RasterMask &mask, NodeType type)
            : type(type), mask(mask), children() {};

        bool get( uint x, uint y, uint size ) const;
        void set( uint x, uint y, bool value, uint size );

        Node & operator = ( const Node & s );
        ~Node();

        void dump( std::ostream & f ) const;
        void load( std::istream & f );

        void dump( imgproc::bitfield::RasterMask &m, uint x, uint y
                   , uint size ) const;

        /** Called from RasterMask::forEachQuad */
        template <typename Op>
        void descend(uint x, uint y, uint size, const Op &op
                     , Filter filter) const;

        /** Inverts node (black -> white, white->black, gray is recursed down).
         */
        void invert();

        void merge(const Node &other);

        void intersect(const Node &other);

        void subtract(const Node &other);

        void coarsen(uint size, const uint threshold);

        /** Constracts node if all children are either white or black.
         */
        void contract();

        NodeType type;
        RasterMask &mask;
        NodeChildren *children;
    };

    struct NodeChildren {
        NodeChildren(RasterMask &mask)
            : ul(mask), ur(mask), ll(mask), lr(mask)
        {}

        NodeChildren(RasterMask &mask, NodeType type)
            : ul(mask, type), ur(mask, type)
            , ll(mask, type), lr(mask, type)
        {}

        Node ul, ur, ll, lr;
    };

    NodeChildren* malloc();
    NodeChildren* malloc(NodeType type);
    void free(NodeChildren *&children);

    uint sizeX_, sizeY_;
    uint quadSize_;
    ulong count_;
    Node root_;
};

} } // namespace imgproc::quadtree

#include "./inline/quadtree.hpp"

#endif // imgproc_rastermask_quadtree_hpp_included_
