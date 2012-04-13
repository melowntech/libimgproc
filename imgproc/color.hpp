/**
 * @file color.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Generic color representation
 */

#ifndef IMGPROC_COLOR_HPP
#define IMGPROC_COLOR_HPP

#include <boost/numeric/ublas/vector.hpp>
#include <boost/gil/gil_all.hpp>
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
