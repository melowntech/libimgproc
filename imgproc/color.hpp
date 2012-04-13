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

class YCCColor_t;

class RGBColor_t: public ublas::vector<float> {

public :
    RGBColor_t( float r = 0, float g = 0, float b = 0 );
    RGBColor_t( const ublas::vector<float> & c );
    RGBColor_t( const YCCColor_t & c );
    RGBColor_t( const gil::rgb8_pixel_t & p );
    gil::rgb8_pixel_t rgbpixel() const;
};

class YCCColor_t : public ublas::vector<float> {

public :
    YCCColor_t( float y = 0, float cb = 0, float cr = 0 );
    YCCColor_t( const ublas::vector<float> & c );
    YCCColor_t( const RGBColor_t & c );
    YCCColor_t( const gil::rgb8_pixel_t & yccpixel );
    YCCColor_t( const gil::rgb32f_pixel_t & yccpixel );
    gil::rgb8_pixel_t yccpixel() const;
};

/**
 * Return an indication of chromatic difference between two YCCColors
 */

float ccDiff( const YCCColor_t & color1, const YCCColor_t & color2 );


} // namespace imgproc

#endif // IMGPROC_COLOR_HPP
