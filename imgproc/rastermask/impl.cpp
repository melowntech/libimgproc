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

namespace imgproc {

namespace bitfield {

namespace {
    const char BF_RASTERMASK_IO_MAGIC[5] = { 'R', 'M', 'A', 'S', 'K' };

    using utility::binaryio::read;
    using utility::binaryio::write;
}

void RasterMask::dump(std::ostream &f) const
{
    write(f, BF_RASTERMASK_IO_MAGIC); // 5 bytes
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved

    uint32_t width(size_.width);
    uint32_t height(size_.height);
    write(f, width);
    write(f, height);

    write(f, mask_.get(), bytes_);
}

void RasterMask::load(std::istream &f)
{
    char magic[5];
    read(f, magic);

    if (std::memcmp(magic, BF_RASTERMASK_IO_MAGIC,
                    sizeof(BF_RASTERMASK_IO_MAGIC))) {
        LOGTHROW(err2, std::runtime_error) << "RasterMask has wrong magic.";
    }

    uint8_t reserved1, reserved2, reserved3;
    read(f, reserved1); // reserved
    read(f, reserved2); // reserved
    read(f, reserved3); // reserved

    uint32_t width, height;
    read(f, width);
    read(f, height);

    size_.width = width;
    size_.height = height;
    bytes_ = (size_.height * size_.width + 7) >> 3;
    mask_.reset(new std::uint8_t[bytes_]);

    read(f, mask_.get(), bytes_);

    resetTrail();

    // compute count_
    count_ = std::accumulate
        (mask_.get(), mask_.get() + bytes_, 0u
         , [](unsigned int value, std::uint8_t byte) {
            return (value + (byte & 0x01) + ((byte & 0x02) >> 1)
                    + ((byte & 0x04) >> 2) + ((byte & 0x08) >> 3)
                    + ((byte & 0x10) >> 4) + ((byte & 0x20) >> 5)
                    + ((byte & 0x40) >> 6) + ((byte & 0x80) >> 7));
        });
}

} // namespace bitfield


/******************************************************************************/

namespace quadtree {

namespace {
    const char QT_RASTERMASK_IO_MAGIC[5] = { 'Q', 'M', 'A', 'S', 'K' };

    using utility::binaryio::read;
    using utility::binaryio::write;
}


RasterMask::RasterMask( uint sizeX, uint sizeY, const InitMode mode )
    : sizeX_( sizeX ), sizeY_( sizeY ), root_( *this )
{
    quadSize_ = 1;
    while (quadSize_ < sizeX_ || quadSize_ < sizeY_) {
        quadSize_ *= 2;
    }

    switch ( mode ) {

        case EMPTY :
            root_.type = BLACK;
            count_ = 0;
            break;

        case FULL :
        default :
            root_.type = WHITE;
            count_ = sizeX * sizeY;
            break;
    }
}

RasterMask::RasterMask( const math::Size2 & size, const InitMode mode )
    : sizeX_( size.width ), sizeY_( size.height ), root_( * this )
{
    quadSize_ = 1;
    while (quadSize_ < sizeX_ || quadSize_ < sizeY_) {
        quadSize_ *= 2;
    }

    switch ( mode ) {

        case EMPTY :
            root_.type = BLACK;
            count_ = 0;
            break;

        case FULL :
        default :
            root_.type = WHITE;
            count_ = sizeX_ * sizeY_;
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

void RasterMask::subtract( const RasterMask & op ) {

    for ( uint i = 0; i < sizeX_; i++ )
        for ( uint j = 0; j < sizeY_; j++ )
            if ( op.get( i, j ) ) set( i, j, false );
}

void RasterMask::invert()
{
    root_.invert();
#if 0
    for ( uint i = 0; i < sizeX_; i++ )
        for ( uint j = 0; j < sizeY_; j++ )
            set( i, j, ! get( i, j ) );
#endif
}

bool RasterMask::get( int x, int y ) const {

    if ( x < 0 || x >= (int) sizeX_ || y < 0 || y >= (int) sizeY_ ) return false;
    return root_.get( (uint) x, (uint) y, quadSize_ );
}

void RasterMask::set( int x, int y, bool value ) {

    if ( x < 0 || x >= (int) sizeX_ || y < 0 || y >= (int) sizeY_ ) return;
    root_.set( (uint) x, (uint ) y, value, quadSize_ );
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
    f.write( reinterpret_cast<const char *>( & count_ ), sizeof( uint ) );

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
    f.read( reinterpret_cast<char *>( & count_ ), sizeof( uint ) );

    root_.load( f );
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


/* class RasterMask::Node */

RasterMask::Node::~Node() {

    if ( type == GRAY ) {
        delete ul; delete ll; delete ur; delete lr;
    }
}

bool RasterMask::Node::get( uint x, uint y, uint size ) const {

    uint split = size >> 1;

    switch ( type ) {
        case WHITE :
            return true;
            break;
        case BLACK :
            return false;
            break;
        case GRAY :
            if ( x < split )
                if ( y < split )
                    return( ul->get( x, y, split ) );
                else
                    return( ll->get( x, y - split, split ) );
            else
                if ( y < split )
                    return( ur->get( x - split, y, split ) );
                else
                    return( lr->get( x - split, y - split, split ) );
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

        ul = new Node( mask ); ul->type = type;
        ll = new Node( mask ); ll->type = type;
        ur = new Node( mask ); ur->type = type;
        lr = new Node( mask ); lr->type = type;

        type = GRAY;
    }

    // process
    if ( type == BLACK )
        if ( value ) { type = WHITE; mask.count_++; }

    if ( type == WHITE )
        if ( ! value ) { type = BLACK; mask.count_--; }

    if ( type == GRAY ) {

        if ( x < split )
            if ( y < split )
                ul->set( x, y, value, split );
            else
                ll->set( x, y - split, value, split );
        else
            if ( y < split )
                ur->set( x - split, y, value, split );
            else
                lr->set( x - split, y - split, value, split );
    }

    // contract node if possible
    if ( type == GRAY ) {

        if ( ul->type == WHITE && ll->type == WHITE && ur->type == WHITE &&
            lr->type == WHITE ) {

            delete ul; delete ll; delete ur; delete lr;
            type = WHITE;
        }
        else
        if ( ul->type == BLACK && ll->type == BLACK && ur->type == BLACK &&
            lr->type == BLACK ) {

            delete ul; delete ll; delete ur; delete lr;
            type = BLACK;
        }
    }
}

RasterMask::Node & RasterMask::Node::operator = (
    const RasterMask::Node & s ) {

    // no self-assignment
    if ( this == &s ) return *this;

    // clean
    if ( type == GRAY ) {
        delete ul; delete ll; delete ur; delete lr;
    }

    // assign
    type = s.type;

    if ( type == GRAY ) {

        ul = new Node( mask );
        ll = new Node( mask );
        ur = new Node( mask );
        lr = new Node( mask );

        *ul = *(s.ul); *ll = *(s.ll); *ur = *(s.ur); *lr = *(s.lr);
    }

    // return
    return * this;
}

void RasterMask::Node::dump( std::ostream & f ) const
{
    uint8_t c = type;
    write(f, c);

    if ( type == GRAY ) {

        ul->dump( f );
        ur->dump( f );
        ll->dump( f );
        lr->dump( f );
    }
}

void RasterMask::Node::load( std::istream & f )
{
    uint8_t c;
    read(f, c);
    type = static_cast<NodeType>(c);

    if ( type == GRAY ) {

        ul = new Node( mask );
        ur = new Node( mask );
        ll = new Node( mask );
        lr = new Node( mask );

        ul->load( f );
        ur->load( f );
        ll->load( f );
        lr->load( f );
    }
}

imgproc::bitfield::RasterMask RasterMask::asBitfield() const
{
    LOG(info4) << "Converting raster mask from quad-tree based representation";
    imgproc::bitfield::RasterMask m
        (sizeX_, sizeY_, imgproc::bitfield::RasterMask::EMPTY);
    root_.dump(m, 0, 0, quadSize_);
    LOG(info4) << "RasterMask: " << m.size() << " vs " << count_;

    return m;
}

void RasterMask::Node::dump(imgproc::bitfield::RasterMask &m
                              , uint x, uint y, uint size)
    const
{
    uint split = size / 2;

    switch ( type ) {
    case WHITE:
        {
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
        }
        return;

    case BLACK :
        return;

    case GRAY :
        ul->dump( m, x, y, split );
        ll->dump( m, x, y + split, split );
        ur->dump( m, x + split, y, split );
        lr->dump( m, x + split, y + split, split );
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
        ul->invert();
        ll->invert();
        ur->invert();
        lr->invert();
        break;
    }
}

} // namespace quadtree

} // namespace imgproc
