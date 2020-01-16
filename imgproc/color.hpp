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
 * @file color.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Generic color representation
 */

#ifndef IMGPROC_COLOR_HPP
#define IMGPROC_COLOR_HPP

#include <boost/numeric/ublas/vector.hpp>
#include "math/boost_gil_all.hpp"
#include <boost/numeric/ublas/io.hpp>

namespace imgproc {

namespace ublas = boost::numeric::ublas;
namespace gil = boost::gil;

class YCCColor;

class RGBColor: public ublas::vector<float> {

public :
    RGBColor( float r = 0, float g = 0, float b = 0 );
    RGBColor( const ublas::vector<float> & c );
    RGBColor( const YCCColor & c );
    RGBColor( const gil::rgb8_pixel_t & p );
    gil::rgb8_pixel_t rgbpixel() const;
};

class YCCColor : public ublas::vector<float> {

public :
    YCCColor( float y = 0, float cb = 0, float cr = 0 );
    YCCColor( const ublas::vector<float> & c );
    YCCColor( const RGBColor & c );
    YCCColor( const gil::rgb8_pixel_t & yccpixel );
    YCCColor( const gil::rgb32f_pixel_t & yccpixel );
    gil::rgb8_pixel_t yccpixel() const;
};

/**
 * Return an indication of chromatic difference between two YCCColors
 */

float ccDiff( const YCCColor & color1, const YCCColor & color2 );


} // namespace imgproc

#endif // IMGPROC_COLOR_HPP
