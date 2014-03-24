/**
 * @file rastermask/quadtree.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Raster mask (bitmap).
 */

#ifndef imgproc_rastermask_quadtree_hpp_included_
#define imgproc_rastermask_quadtree_hpp_included_

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iosfwd>

#include <boost/scoped_array.hpp>

#include "math/geometry_core.hpp"

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
    void subtract( const RasterMask & op );

    /** obtain mask value at given pos. */
    bool get( int x, int y ) const;

    /** set mask value at given pos. */
    void set( int x, int y, bool value = true );

    /** test if a given pixel is a boundary pixel (neighboring unset
        pixel in mask */
    bool onBoundary( int x, int y ) const;

    /** return mask size (number of white pixels) */
    uint count() const { return count_; }

    /** return total number of pixels */
    uint capacity() const { return sizeX_ * sizeY_; }

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

private :

    enum NodeType { WHITE, BLACK, GRAY };

    struct Node
    {
        Node( RasterMask & mask ) : type( BLACK ), mask( mask ) {};
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

        NodeType type;
        Node * ul, * ur, * ll, *lr;
        RasterMask & mask;
    };

    uint sizeX_, sizeY_;
    uint quadSize_;
    uint  count_;
    Node root_;
};

} } // namespace imgproc::quadtree

#include "./inline/quadtree.hpp"

#endif // imgproc_rastermask_quadtree_hpp_included_
