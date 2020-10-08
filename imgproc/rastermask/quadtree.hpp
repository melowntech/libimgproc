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

#include "bitfieldfwd.hpp"

/**** quad-tree version of rastermask ****/

namespace imgproc { namespace mappedqtree {
class RasterMask;
} } // imgproc::mappedqtree

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
    RasterMask( unsigned int sizeX, unsigned int sizeY, const InitMode mode );

    /** intialize mask */
    RasterMask( const math::Size2 & size, const InitMode mode );

    /** initialize a mask of the same order, optionally copying mask. */
    RasterMask( const RasterMask & mask, const InitMode mode = SOURCE );

    /** Initialize mask from other mask's subtree at given coordinates.
     */
    RasterMask(const RasterMask &other, const math::Size2 &size
               , unsigned int depth, unsigned int x, unsigned int y);

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

    /** set mask value at whole quad.
     *
     *  \param depth depth in tree, root starts at 0
     *  \param x horizontal index in quads at given depth
     *  \param y vertical index in quads at given depth
     *  \param value value to se to
     */
    void setQuad(int depth, int x, int y, bool value = true);

    /** Set subtree from other mask. Subtree is clipped at maximum mask depth.
     *
     *  \param depth depth in tree, root starts at 0
     *  \param x horizontal index in quads at given depth
     *  \param y vertical index in quads at given depth
     *  \param mask to source data from
     */
    void setSubtree(int depth, int x, int y, const RasterMask &mask);

    /** Resets whole mask to given value. */
    void reset(bool value = true);

    /** test if a given pixel is a boundary pixel (neighboring unset
        pixel in mask */
    bool onBoundary( int x, int y ) const;

    /** return mask size (number of white pixels) */
    unsigned long long count() const { return count_; }

    /** return total number of pixels */
    unsigned long long capacity() const {
        return (unsigned long long)(sizeX_) * (unsigned long long)(sizeY_);
    }

    /** test mask for emptiness */
    bool empty() const { return count_ == 0; }

    bool full() const { return count_ == capacity(); }

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

    /** Runs op(x, y, xsize, ysize, boost::tribool) for each black/white/gray
     *  quad (gray is marked by indeterminate value). Tree descent is terminated
     *  at given tree depth.
     */
    template <typename Op>
    void forEachQuad(unsigned int depth, const Op &op) const;

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
    void coarsen(const unsigned int threshold = 2);

    /** Returns new raster mask that created from subtreee at given quad.
     *
     * Quad is addressed by depth from root and index in grid at given depth.
     *
     * Mask is assigned given size.
     */
    RasterMask subTree(const math::Size2 &size
                       , unsigned int depth, unsigned int x, unsigned int y) const;

    /** Returns maximal depth of tree.
     */
    unsigned int depth() const { return depth_; }

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

        bool get( unsigned int x, unsigned int y, unsigned int size ) const;
        void set( unsigned int x, unsigned int y, bool value, unsigned int size );
        void setQuad(unsigned int depth, unsigned int x, unsigned int y, bool value, unsigned int size);
        void setSubtree(unsigned int depth, unsigned int x, unsigned int y, const RasterMask &other
                        , unsigned int size);

        const Node* findSubtree(unsigned int depth, unsigned int x, unsigned int y, unsigned int size) const;

        Node & operator = ( const Node & s );
        ~Node();

        void dump( std::ostream & f ) const;
        void load( std::istream & f );

        void dump2( std::ostream & f ) const;

        void dump( imgproc::bitfield::RasterMask &m, unsigned int x, unsigned int y
                   , unsigned int size ) const;

        /** Called from RasterMask::forEachQuad */
        template <typename Op>
        void descend(unsigned int x, unsigned int y, unsigned int size, const Op &op
                     , Filter filter) const;

        /** Called from RasterMask::forEachQuad */
        template <typename Op>
        void descend(unsigned int depth, unsigned int x, unsigned int y, unsigned int size, const Op &op) const;

        /** Inverts node (black -> white, white->black, gray is recursed down).
         */
        void invert();

        void merge(const Node &other);

        void intersect(const Node &other);

        void subtract(const Node &other);

        void coarsen(unsigned int size, const unsigned int threshold);

        /** Contracts node if all children are either white or black.
         */
        void contract();

        /** Finds quad in given subtree.
         */
        const Node& find(unsigned int depth, unsigned int x, unsigned int y) const;

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
    const Node* findSubtree(int depth, int x, int y) const;

    unsigned int sizeX_, sizeY_;
    unsigned int depth_;
    unsigned int quadSize_;
    unsigned long long count_;
    Node root_;

    /** Needed for mappedqtree::RasterMask creation.
     */
    friend class mappedqtree::RasterMask;
};

void resizeMask(const RasterMask &src, RasterMask &dst);

} } // namespace imgproc::quadtree

#include "inline/quadtree.hpp"

#endif // imgproc_rastermask_quadtree_hpp_included_
