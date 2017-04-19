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
/*
 * color.cpp
 */

#include "color.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <cmath>


namespace imgproc {

/* class RGBColor */

RGBColor::RGBColor( float r, float g, float b )
    : ublas::vector<float>(3) {
    (*this)(0) = r; (*this)(1) = g; (*this)(2) = b;
}

RGBColor::RGBColor( const ublas::vector<float> & c )
    : ublas::vector<float>( c ) {}

RGBColor::RGBColor( const YCCColor & c )
    : ublas::vector<float>(3) {

    //assert( c(0) <= 1.0 && c(0) >= 0.0 );

    ublas::matrix<float> a( 3, 3 );
    a( 0, 0 ) = 1.0; a( 0, 1 ) = 9.2674E-4; a( 0, 2 ) = 1.4017;
    a( 1, 0 ) = 1.0; a( 1, 1 ) =  -0.34370; a( 1, 2 ) = -0.7142;
    a( 2, 0 ) = 1.0; a( 2, 1 ) = 1.7722; a( 2, 2 ) = 9.9022E-4;

    *this = RGBColor( ublas::prod( a, c ) );

    // clip to fit into rgb gamut
    ublas::vector<float> yccnochroma( 3 );
    yccnochroma(0) = c(0); yccnochroma(1) = yccnochroma(2) = 0.0;
    ublas::vector<float> nochroma = ublas::prod( a, yccnochroma );

    if ( (*this)(0) < 0.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 0.0 - nochroma(0) ) / ( (*this)(0) - nochroma(0) );
        *this = RGBColor( nochroma + u * diff );
    }

    if ( (*this)(0) > 1.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 1.0 - nochroma(0) ) / ( (*this)(0) - nochroma(0) );
        *this = RGBColor( nochroma + u * diff );
    }

    if ( (*this)(1) < 0.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 0.0 - nochroma(1) ) / ( (*this)(1) - nochroma(1) );
        *this = RGBColor( nochroma + u * diff );
    }

    if ( (*this)(1) > 1.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 1.0 - nochroma(1) ) / ( (*this)(1) - nochroma(1) );
        *this = RGBColor( nochroma + u * diff );
    }

    if ( (*this)(2) < 0.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 0.0 - nochroma(2) ) / ( (*this)(2) - nochroma(2) );
        *this = RGBColor( nochroma + u * diff );
    }

    if ( (*this)(2) > 1.0 ) {
        ublas::vector<float> diff = *this - nochroma;
        float u = ( 1.0 - nochroma(2) ) / ( (*this)(2) - nochroma(2) );
        *this = RGBColor( nochroma + u * diff );
    }
}


gil::rgb8_pixel_t RGBColor::rgbpixel() const {

    return gil::rgb8_pixel_t(
        round( 0xff * (*this)(0) ),
        round( 0xff * (*this)(1) ),
        round( 0xff * (*this)(2) ) );
}

RGBColor::RGBColor( const gil::rgb8_pixel_t & p )
    : ublas::vector<float>(3) {

    (*this)(0) = (float) p[0] / 0xff;
    (*this)(1) = (float) p[1] / 0xff;
    (*this)(2) = (float) p[2] / 0xff;
}

/* class YCCColor */

YCCColor::YCCColor( float y, float cb, float cr )
    : ublas::vector<float>(3) {
    (*this)(0) = y; (*this)(1) = cb; (*this)(2) = cr;
}

YCCColor::YCCColor( const ublas::vector<float> & c )
    : ublas::vector<float>( c ) {}

YCCColor::YCCColor( const RGBColor & c )
    : ublas::vector<float>(3) {

    /*assert( c(0) <= 1.0 && c(0) >= 0.0 );
    assert( c(1) <= 1.0 && c(1) >= 0.0 );
    assert( c(2) <= 1.0 && c(2) >= 0.0 );*/

    ublas::matrix<float> a( 3, 3 );
    a( 0, 0 ) = 0.299; a( 0, 1 ) = 0.587; a( 0, 2 ) = 0.114;
    a( 1, 0 ) = -0.169; a( 1, 1 ) = -0.331; a( 1, 2 ) = 0.500;
    a( 2, 0 ) = 0.500; a( 2, 1 ) = -0.419; a( 2, 2 ) = -0.081;

    //std::cout << ublas::prod( a, c );

    *this = YCCColor( ublas::prod( a, c ) );

    //std::cout << " =? " << *this << std::endl;
}

YCCColor::YCCColor( const gil::rgb8_pixel_t & yccpixel )
    : ublas::vector<float>(3) {

    (*this)(0) = ( (float) yccpixel[0] / 0xff ) - 0.5;
    (*this)(1) = ( (float) yccpixel[1] / 0xff ) - 0.5;
    (*this)(2) = ( (float) yccpixel[2] / 0xff ) - 0.5;
}

YCCColor::YCCColor( const gil::rgb32f_pixel_t & yccpixel )
    : ublas::vector<float>(3) {

    (*this)(0) = ( yccpixel[0] / 0xff ) - 0.5;
    (*this)(1) = ( yccpixel[1] / 0xff ) - 0.5;
    (*this)(2) = ( yccpixel[2] / 0xff ) - 0.5;
}


gil::rgb8_pixel_t YCCColor::yccpixel() const {

    return gil::rgb8_pixel_t(
        (int) round( 0xff * (*this)(0) ),
        (int) round( 0xff * ( (*this)(1) + 0.5 ) ),
        (int) round( 0xff * ( (*this)(2) + 0.5 ) ) );
}

float ccDiff( const YCCColor & color1, const YCCColor & color2 ) {

    float hue1, hue2;

    hue1 = atan2( color1( 2 ), color1( 1 ) );

    hue2 = atan2( color2( 2 ), color2( 1 ) );

    return std::min( fabs( hue1 - hue2 ), 2 * M_PI - fabs( hue1 - hue2 ) );

    //return sqr( color2( 1 ) - color1( 1 ) ) + sqr( color2( 2 ) - color1( 2 ) );
}

} // namespace imgproc 
