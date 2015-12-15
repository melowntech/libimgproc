/**
 * @file rastermask.cpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (bitmap).
 */

#include <iostream>
#include <stdexcept>
#include <numeric>

#include "dbglog/dbglog.hpp"
#include "utility/binaryio.hpp"

#include "bitfield.hpp"
#include "quadtree.hpp"

namespace imgproc { namespace quadtree {

namespace {
    const char QT_RASTERMASK_IO_MAGIC[5] = { 'Q', 'M', 'A', 'S', 'K' };

    using utility::binaryio::read;
    using utility::binaryio::write;

    uint computeQuadSize(uint sizeX, uint sizeY)
    {
        uint quadSize = 1;
        while ((quadSize < sizeX) || (quadSize < sizeY)) {
            quadSize <<= 1;
        }
        return quadSize;
    }
}


RasterMask::RasterMask( uint sizeX, uint sizeY, const InitMode mode )
    : sizeX_( sizeX ), sizeY_( sizeY )
    , quadSize_(computeQuadSize(sizeX_, sizeY_))
    , root_( *this )
{
    switch ( mode ) {
    case EMPTY :
        root_.type = BLACK;
        count_ = 0;
        break;

    case FULL :
    default :
        root_.type = WHITE;
        count_ = ulong(sizeX_) * ulong(sizeY_);
        break;
    }
}

RasterMask::RasterMask( const math::Size2 & size, const InitMode mode )
    : sizeX_( size.width ), sizeY_( size.height )
    , quadSize_(computeQuadSize(sizeX_, sizeY_))
    , root_( * this )
{
    switch ( mode ) {
    case EMPTY :
        root_.type = BLACK;
        count_ = 0;
        break;

    case FULL :
    default :
        root_.type = WHITE;
        count_ = ulong(sizeX_) * ulong(sizeY_);
        break;
    }
}

RasterMask::RasterMask( const RasterMask & mask, const InitMode mode )
    : sizeX_( mask.sizeX_ ), sizeY_( mask.sizeY_ ), quadSize_( mask.quadSize_ ),
      root_( *this )
{
    switch ( mode ) {
    case EMPTY:
        root_.type = BLACK;
        break;

    case FULL:
        root_.type = WHITE;
        break;

    case SOURCE:
    default:
        count_ = mask.count_;
        root_ = mask.root_;
        break;
    }
}

void RasterMask::invert()
{
    root_.invert();
}

bool RasterMask::get( int x, int y ) const {

    if ( x < 0 || x >= (int) sizeX_ || y < 0 || y >= (int) sizeY_ ) return false;
    return root_.get( (uint) x, (uint) y, quadSize_ );
}

bool RasterMask::getClamped( int x, int y ) const {

    if ( x < 0 ) x = 0;
    else if ( x >= (int) sizeX_) x = sizeX_ - 1;

    if ( y < 0 ) y = 0;
    else if ( y >= (int) sizeY_ ) y = sizeY_ - 1;

    return root_.get( (uint) x, (uint) y, quadSize_ );
}

void RasterMask::set( int x, int y, bool value ) {

    if ( x < 0 || x >= (int) sizeX_ || y < 0 || y >= (int) sizeY_ ) return;
    root_.set( (uint) x, (uint ) y, value, quadSize_ );
}

void RasterMask::reset(bool value)
{
    // set root to given value
    root_ = Node(*this, value ? NodeType::WHITE : NodeType::BLACK);
}

bool RasterMask::onBoundary( int x, int y ) const {

    if ( ! get( x, y ) ) return false;

    for ( int i = -1; i <= 1; i++ )
        for ( int j = -1; j <= 1; j++ ) {

        if ( ! ( i == 0 && j == 0 ) && x + i >= 0 && y +j >= 0
            && x + i <= (int) sizeX_ - 1 && y + j <= (int) sizeY_ - 1 )
            if ( ! get( x + i , y + j ) ) return true;
    }

    return false;
}

void RasterMask::dump( std::ostream & f ) const
{
    write(f, QT_RASTERMASK_IO_MAGIC); // 5 bytes
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved

    f.write( reinterpret_cast<const char *>( & sizeX_ ), sizeof( uint ) );
    f.write( reinterpret_cast<const char *>( & sizeY_ ), sizeof( uint ) );
    f.write( reinterpret_cast<const char *>( & quadSize_ ), sizeof( uint ) );
    // count ignored
    std::uint32_t count(0);
    f.write( reinterpret_cast<const char *>( & count ), sizeof( count ) );

    root_.dump( f );
}

void RasterMask::load( std::istream & f )
{
    char magic[5];
    read(f, magic);

    if (std::memcmp(magic, QT_RASTERMASK_IO_MAGIC,
                    sizeof(QT_RASTERMASK_IO_MAGIC))) {
        LOGTHROW(err2, std::runtime_error) << "RasterMask has wrong magic.";
    }

    uint8_t reserved1, reserved2, reserved3;
    read(f, reserved1); // reserved
    read(f, reserved2); // reserved
    read(f, reserved3); // reserved

    f.read( reinterpret_cast<char *>( & sizeX_ ), sizeof( uint ) );
    f.read( reinterpret_cast<char *>( & sizeY_ ), sizeof( uint ) );
    f.read( reinterpret_cast<char *>( & quadSize_ ), sizeof( uint ) );
    std::uint32_t count(0);
    f.read( reinterpret_cast<char *>( & count ), sizeof( count ) );

    root_.load( f );
    recount();
}

RasterMask & RasterMask::operator = ( const RasterMask & op )
{
    if ( & op == this ) return *this;

    sizeX_ = op.sizeX_;
    sizeY_ = op.sizeY_;
    quadSize_ = op.quadSize_;
    root_ = op.root_;
    count_ = op.count_;

    return *this;
}


RasterMask::NodeChildren* RasterMask::malloc()
{
    return new NodeChildren(*this);
}

RasterMask::NodeChildren* RasterMask::malloc(NodeType type)
{
    return new NodeChildren(*this, type);
}

void RasterMask::free(NodeChildren *&children)
{
    delete children;
    children = 0x0;
}

void RasterMask::merge(const RasterMask &other, bool checkDimensions)
{
    if (checkDimensions) {
        if ((sizeX_ != other.sizeX_) || (sizeY_ != other.sizeY_)) {
            LOGTHROW(err1, std::runtime_error)
                << "Attempt to merge in data from mask with diferent "
                "dimensions.";
        }
    }
    root_.merge(other.root_);
    recount();
}

void RasterMask::intersect(const RasterMask &other)
{
    if ((sizeX_ != other.sizeX_) || (sizeY_ != other.sizeY_)) {
        LOGTHROW(err1, std::runtime_error)
            << "Attempt to intersect with data from mask with diferent "
            "dimensions.";
    }
    root_.intersect(other.root_);
    recount();
}

void RasterMask::subtract(const RasterMask &other)
{
    if ((sizeX_ != other.sizeX_) || (sizeY_ != other.sizeY_)) {
        LOGTHROW(err1, std::runtime_error)
            << "Attempt to subtract data from mask with diferent "
            "dimensions.";
    }
    root_.subtract(other.root_);
    recount();
}

void RasterMask::coarsen(const uint threshold)
{
    // sanity check
    if (threshold < 2) {
        return;
    }

    if (threshold > quadSize_) {
        LOGTHROW(err1, std::runtime_error)
            << "Attempt to coarsen to bigger quad than is size of rastermask.";
    }

    root_.coarsen(quadSize_, threshold);
    recount();
}

void RasterMask::recount()
{
    ulong count(0);
    forEachQuad([&count](uint, uint, uint xsize, uint ysize, bool) {
            count += ulong(xsize) * ulong(ysize);
        }, Filter::white);
    count_ = count;
}

/* class RasterMask::Node */

RasterMask::Node::~Node() {
    mask.free(children);
}

bool RasterMask::Node::get( uint x, uint y, uint size ) const {

    uint split = size >> 1;

    switch ( type ) {
    case WHITE :
        return true;

    case BLACK :
        return false;

    case GRAY :
        if ( x < split ) {
            if ( y < split ) {
                return( children->ul.get( x, y, split ) );
            } else {
                return( children->ll.get( x, y - split, split ) );
            }
        } else {
            if ( y < split ) {
                return( children->ur.get( x - split, y, split ) );
            } else {
                return( children->lr.get( x - split, y - split, split ) );
            }
        }
        break;

    default :
        return false;
    }
}

void RasterMask::Node::set( uint x, uint y, bool value, uint size )
{
    uint split = size >> 1;

    // split node if necessarry
    if ( ( ( type == BLACK && value ) || ( type == WHITE && ! value  ) )
        && size > 1 ) {
        children = mask.malloc(type);
        type = GRAY;
    }

    // process
    if ( type == BLACK ) {
        if ( value ) { type = WHITE; mask.count_++; }
    } else if ( type == WHITE ) {
        if ( ! value ) { type = BLACK; mask.count_--; }
    } else if ( type == GRAY ) {
        if ( x < split ) {
            if ( y < split ) {
                children->ul.set( x, y, value, split );
            } else {
                children->ll.set( x, y - split, value, split );
            }
        } else {
            if ( y < split ) {
                children->ur.set( x - split, y, value, split );
            } else {
                children->lr.set( x - split, y - split, value, split );
            }
        }
    }

    // contract node if possible
    contract();
}

void RasterMask::Node::contract()
{
    if (type != GRAY) {
        return;
    }

    if (children->ul.type == WHITE && children->ll.type == WHITE
        && children->ur.type == WHITE && children->lr.type == WHITE )
    {
        mask.free(children);
        type = WHITE;
    } else if (children->ul.type == BLACK && children->ll.type == BLACK
               && children->ur.type == BLACK && children->lr.type == BLACK)
    {
        mask.free(children);
        type = BLACK;
    }
}

RasterMask::Node & RasterMask::Node::operator = (
    const RasterMask::Node & s ) {

    // no self-assignment
    if ( this == &s ) return *this;

    // clean
    if ( type == GRAY ) {
        mask.free(children);
    }

    // assign
    type = s.type;

    if ( type == GRAY ) {
        children = mask.malloc();
        children->ul = s.children->ul;
        children->ll = s.children->ll;
        children->ur = s.children->ur;
        children->lr = s.children->lr;
    }

    // return
    return * this;
}

void RasterMask::Node::dump( std::ostream & f ) const
{
    uint8_t c = type;
    write(f, c);

    if ( type == GRAY ) {

        children->ul.dump( f );
        children->ur.dump( f );
        children->ll.dump( f );
        children->lr.dump( f );
    }
}

void RasterMask::Node::load( std::istream & f )
{
    uint8_t c;
    read(f, c);
    type = static_cast<NodeType>(c);

    if ( type == GRAY ) {
        children = mask.malloc();

        children->ul.load( f );
        children->ur.load( f );
        children->ll.load( f );
        children->lr.load( f );
    }
}

imgproc::bitfield::RasterMask RasterMask::asBitfield() const
{
    LOG(info1) << "Converting raster mask from quad-tree based representation";
    imgproc::bitfield::RasterMask m
        (sizeX_, sizeY_, imgproc::bitfield::RasterMask::EMPTY);
    root_.dump(m, 0, 0, quadSize_);
    LOG(info1) << "RasterMask: " << m.size() << " vs " << count_;

    return m;
}

void RasterMask::Node::dump(imgproc::bitfield::RasterMask &m
                              , uint x, uint y, uint size)
    const
{
    uint split = size / 2;

    switch ( type ) {
    case WHITE: {
        // fill in quad
        uint ex(x + size);
        uint ey(y + size);
        if (ex > mask.sizeX_) { ex = mask.sizeX_; };
        if (ey > mask.sizeY_) { ey = mask.sizeY_; };

        for (uint j(y); j < ey; ++j) {
            for (uint i(x); i < ex; ++i) {
                m.set(i, j);
            }
        }
        return;
    }

    case BLACK :
        return;

    case GRAY :
        children->ul.dump( m, x, y, split );
        children->ll.dump( m, x, y + split, split );
        children->ur.dump( m, x + split, y, split );
        children->lr.dump( m, x + split, y + split, split );
        break;
    }
}

void RasterMask::Node::invert()
{
    switch (type) {
    case WHITE:
        type = BLACK;
        return;

    case BLACK :
        type = WHITE;
        return;

    case GRAY :
        children->ul.invert();
        children->ll.invert();
        children->ur.invert();
        children->lr.invert();
        break;
    }
}

void RasterMask::Node::merge(const Node &other)
{
    if ((type == NodeType::WHITE) || (other.type == NodeType::BLACK)) {
        // merge(WHITE, anything) = WHITE (keep)
        // merge(anything, BLACK) = anything (keep)
        return;
    }

    if (other.type == NodeType::WHITE) {
        // merge(anything, WHITE) = WHITE
        *this = other;
        return;
    }

    // OK, other is gray
    if (type == NodeType::BLACK) {
        // merge(BLACK, GRAY) = GRAY
        *this = other;
        return;
    }

    // merge(GRAY, GRAY) = go down
    children->ul.merge(other.children->ul);
    children->ll.merge(other.children->ll);
    children->ur.merge(other.children->ur);
    children->lr.merge(other.children->lr);

    // contract if possible
    contract();
}

void RasterMask::Node::intersect(const Node &other)
{
    if (type == NodeType::BLACK) {
        // intersect(BLACK, anything) = BLACK
        return;
    }

    if (type == NodeType::WHITE) {
        if (other.type == NodeType::BLACK) {
            // intersect(WHITE, BLACK) = BLACK
            type = NodeType::BLACK;
            return;
        } else if (other.type == NodeType::WHITE) {
            // intersect(WHITE, WHITE) = WHITE
            return;
        }

        // intersect(WHITE, GRAY) = GRAY
        *this = other;
        return;
    } else {
        // this is a gray node
        if (other.type == NodeType::BLACK) {
            // intersect(GRAY, BLACK) = BLACK
            mask.free(children);
            type = NodeType::BLACK;
            return;
        } else if (other.type == NodeType::WHITE) {
            // intersect(GRAY, WHITE) = GRAY
            return;
        }
    }

    // intersect(GRAY, GRAY);

    // go down
    children->ul.intersect(other.children->ul);
    children->ll.intersect(other.children->ll);
    children->ur.intersect(other.children->ur);
    children->lr.intersect(other.children->lr);

    // contract if possible
    contract();
}

void RasterMask::Node::subtract(const Node &other)
{
    if ((type == NodeType::BLACK) || (other.type == NodeType::BLACK)) {
        // subtract(BLACK, anything) = BLACK
        // subtract(anything, BLACK) = anything
        return;
    }

    if (type == NodeType::WHITE) {
        if (other.type == NodeType::BLACK) {
            // subtract(WHITE, BLACK) = WHITE
            return;
        } else if (other.type == NodeType::WHITE) {
            // subtract(WHITE, WHITE) = BLACK
            type = NodeType::BLACK;
            return;
        }

        // subtract(WHITE, GRAY) = invert(GRAY)
        *this = other;
        invert();
        return;
    } else {
        // this is gray
        if (other.type == NodeType::WHITE) {
            // subtract(GRAY, WHITE) = BLACK
            mask.free(children);
            type = NodeType::BLACK;
            return;
        }
    }

    // subtract(GRAY, GRAY);

    // go down
    children->ul.subtract(other.children->ul);
    children->ll.subtract(other.children->ll);
    children->ur.subtract(other.children->ur);
    children->lr.subtract(other.children->lr);

    // contract if possible
    contract();
}

void RasterMask::Node::coarsen(uint size, const uint threshold)
{
    if (type != NodeType::GRAY) {
        // keep
        return;
    }

    // gray node
    if (size == threshold) {
        // this is the right spot to cut
        mask.free(children);
        type = WHITE;
        return;
    }

    // need to descend one level down
    size >>= 1;

    children->ul.coarsen(size, threshold);
    children->ll.coarsen(size, threshold);
    children->ur.coarsen(size, threshold);
    children->lr.coarsen(size, threshold);
}

RasterMask::RasterMask(const RasterMask other, const math::Size2 &size
                       , uint depth, uint x, uint y)
    : sizeX_(size.width), sizeY_(size.height)
    , quadSize_(computeQuadSize(sizeX_, sizeY_))
    , root_(*this)
{
    // clone
    root_ = other.root_.find(depth, x, y);
    recount();
}

RasterMask RasterMask::subTree(const math::Size2 &size, uint depth
                               , uint x, uint y) const
{
    return RasterMask(*this, size, depth, x, y);
}

/** Finds quad in given subtree.
 */
const RasterMask::Node& RasterMask::Node::find(uint depth, uint x, uint y)
    const
{
    // if we can descend down (i.e. both tree and depth allow)
    if (depth && (type == GRAY)) {
        // find node to descend
        --depth;
        uint mask(1 << depth);
        auto index(((x & mask) ? 1 : 0) | ((y & mask) ? 2 : 0));

        switch (index) {
        case 0: return children->ul.find(depth, x, y); // upper-left
        case 1: return children->ur.find(depth, x, y); // upper-right
        case 2: return children->ll.find(depth, x, y); // lower-left
        case 3: return children->lr.find(depth, x, y); // lower-right

        default:
            LOGTHROW(err2, std::runtime_error)
                << "Wrong child node index " << index << " calculated"
                " from depth=" << depth << ", x=" << x << ", y=" << y << ".";
        }
    }

    // there is nothing more down there
    return *this;
}

} } // namespace imgproc::quadtree
