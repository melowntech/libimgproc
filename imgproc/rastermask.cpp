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

#include "rastermask.hpp"

namespace imgproc {

namespace bitfield {

namespace {
    const char BF_RASTERMASK_IO_MAGIC[5] = { 'R', 'M', 'A', 'S', 'K' };

    using utility::binaryio::read;
    using utility::binaryio::write;
}

void RasterMask::dump(std::ostream &f)
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


bool RasterMask::empty() const {

    return ( count == 0 );
}

RasterMask::RasterMask( uint sizeX, uint sizeY, const InitMode_t mode )
    : sizeX( sizeX ), sizeY( sizeY ), root( *this )
{
    quadSize = std::max( 1 << uint( ceil( log( sizeX ) / log( 2 ) ) ),
       1 << uint( ceil( log( sizeY ) / log( 2 ) ) ) );

    switch ( mode ) {

        case EMPTY :
            root.type = BLACK;
            count = 0;
            break;

        case FULL :
        default :
            root.type = WHITE;
            count = sizeX * sizeY;
            break;
    }
}

RasterMask::RasterMask( const math::Size2 & size, const InitMode_t mode )
    : sizeX( size.width ), sizeY( size.height ), root( * this ) {

    quadSize = std::max( 1 << uint( ceil( log( sizeX ) / log( 2 ) ) ),
       1 << uint( ceil( log( sizeY ) / log( 2 ) ) ) );

    switch ( mode ) {

        case EMPTY :
            root.type = BLACK;
            count = 0;
            break;

        case FULL :
        default :
            root.type = WHITE;
            count = sizeX * sizeY;
            break;
    }
}

RasterMask::RasterMask( const RasterMask & mask, const InitMode_t mode )
    : sizeX( mask.sizeX ), sizeY( mask.sizeY ), quadSize( mask.quadSize),
      root( *this )
{
    switch ( mode ) {

        case EMPTY:
            root.type = BLACK;
            break;

        case FULL:
            root.type = WHITE;
            break;

        case SOURCE:
        default:
            count = mask.count;
            root = mask.root;
            break;
    }
}

void RasterMask::subtract( const RasterMask & op ) {

    for ( uint i = 0; i < sizeX; i++ )
        for ( uint j = 0; j < sizeY; j++ )
            if ( op.get( i, j ) ) set( i, j, false );
}

void RasterMask::invert() {

    for ( uint i = 0; i < sizeX; i++ )
        for ( uint j = 0; j < sizeY; j++ )
            set( i, j, ! get( i, j ) );
}

bool RasterMask::get( int x, int y ) const {

    if ( x < 0 || x >= (int) sizeX || y < 0 || y >= (int) sizeY ) return false;
    return root.get( (ushort) x, (ushort) y, quadSize );
}

void RasterMask::set( int x, int y, bool value ) {

    if ( x < 0 || x >= (int) sizeX || y < 0 || y >= (int) sizeY ) return;
    root.set( (ushort) x, (ushort ) y, value, quadSize );
}

bool RasterMask::onBoundary( int x, int y ) const {

    if ( ! get( x, y ) ) return false;

    for ( int i = -1; i <= 1; i++ )
        for ( int j = -1; j <= 1; j++ ) {

        if ( ! ( i == 0 && j == 0 ) && x + i >= 0 && y +j >= 0
            && x + i <= (int) sizeX - 1 && y + j <= (int) sizeY - 1 )
            if ( ! get( x + i , y + j ) ) return true;
    }

    return false;
}

void RasterMask::dump( std::ofstream & f )
{
    write(f, QT_RASTERMASK_IO_MAGIC); // 5 bytes
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved
    write(f, uint8_t(0)); // reserved

    f.write( reinterpret_cast<char *>( & sizeX ), sizeof( uint ) );
    f.write( reinterpret_cast<char *>( & sizeY ), sizeof( uint ) );
    f.write( reinterpret_cast<char *>( & quadSize ), sizeof( uint ) );
    f.write( reinterpret_cast<char *>( & count ), sizeof( uint ) );

    root.dump( f );

}

void RasterMask::load( std::ifstream & f )
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

    f.read( reinterpret_cast<char *>( & sizeX ), sizeof( uint ) );
    f.read( reinterpret_cast<char *>( & sizeY ), sizeof( uint ) );
    f.read( reinterpret_cast<char *>( & quadSize ), sizeof( uint ) );
    f.read( reinterpret_cast<char *>( & count ), sizeof( uint ) );

    root.load( f );
}

RasterMask & RasterMask::operator = ( const RasterMask & op )
{
    if ( & op == this ) return *this;

    sizeX = op.sizeX;
    sizeY = op.sizeY;
    quadSize = op.quadSize;
    root = op.root;
    count = op.count;

    return *this;
}


/* class RasterMask::Node_t */

RasterMask::Node_t::~Node_t() {

    if ( type == GRAY ) {
        delete ul; delete ll; delete ur; delete lr;
    }
}

bool RasterMask::Node_t::get( ushort x, ushort y, ushort size ) const {

    ushort split = size >> 1;

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

void RasterMask::Node_t::set( ushort x, ushort y, bool value, ushort size )
{
    ushort split = size >> 1;

    // split node if necessarry
    if ( ( ( type == BLACK && value ) || ( type == WHITE && ! value  ) )
        && size > 1 ) {

        ul = new Node_t( mask ); ul->type = type;
        ll = new Node_t( mask ); ll->type = type;
        ur = new Node_t( mask ); ur->type = type;
        lr = new Node_t( mask ); lr->type = type;

        type = GRAY;
    }

    // process
    if ( type == BLACK )
        if ( value ) { type = WHITE; mask.count++; }

    if ( type == WHITE )
        if ( ! value ) { type = BLACK; mask.count--; }

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

RasterMask::Node_t & RasterMask::Node_t::operator = (
    const RasterMask::Node_t & s ) {

    // no self-assignment
    if ( this == &s ) return *this;

    // clean
    if ( type == GRAY ) {
        delete ul; delete ll; delete ur; delete lr;
    }

    // assign
    type = s.type;

    if ( type == GRAY ) {

        ul = new Node_t( mask );
        ll = new Node_t( mask );
        ur = new Node_t( mask );
        lr = new Node_t( mask );

        *ul = *(s.ul); *ll = *(s.ll); *ur = *(s.ur); *lr = *(s.lr);
    }

    // return
    return * this;
}

void RasterMask::Node_t::dump( std::ofstream & f ) {

    f.write( reinterpret_cast<char *>( & type ), sizeof( NodeType_t ) );

    if ( type == GRAY ) {

        ul->dump( f );
        ur->dump( f );
        ll->dump( f );
        lr->dump( f );
    }
}

void RasterMask::Node_t::load( std::ifstream & f ) {

    f.read( reinterpret_cast<char *>( & type ), sizeof( NodeType_t ) );

    if ( type == GRAY ) {

        ul = new Node_t( mask );
        ur = new Node_t( mask );
        ll = new Node_t( mask );
        lr = new Node_t( mask );

        ul->load( f );
        ur->load( f );
        ll->load( f );
        lr->load( f );
    }
}

imgproc::bitfield::RasterMask RasterMask::asMask() const
{
    LOG(info4) << "Loading raster mask from quad-tree based representation";
    imgproc::bitfield::RasterMask m(sizeX, sizeY, imgproc::RasterMask::EMPTY);
    root.dump(m, 0, 0, quadSize);
    LOG(info4) << "RasterMask: " << m.size() << " vs " << count;

    return m;
}

void RasterMask::Node_t::dump(imgproc::bitfield::RasterMask &m
                              , ushort x, ushort y, ushort size)
    const
{
    ushort split = size / 2;

    switch ( type ) {
    case WHITE:
        {
            // fill in quad
            ushort ex(x + size);
            ushort ey(y + size);
            if (ex > mask.sizeX) { ex = mask.sizeX; };
            if (ey > mask.sizeY) { ey = mask.sizeY; };

            for (ushort j(y); j < ey; ++j) {
                for (ushort i(x); i < ex; ++i) {
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

} // namespace quadtree

} // namespace imgproc
