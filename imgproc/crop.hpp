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
 * @file crop.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Image cropping support.
 */

#ifndef imgproc_crop_included_hpp_
#define imgproc_crop_included_hpp_

#include <iostream>

#if !(defined(_MSC_VER) && defined(__CUDACC__))
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/spirit/include/qi_match_auto.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#endif

#include "math/geometry_core.hpp"

namespace imgproc {

/** Specifies crop area. Similar to math::Viewport2_ but (x, y) specifies pixel
 *  index from image origin (first pixel is 0, 0).
 */
template <typename T>
struct Crop2_ {
    typedef T value_type;
    typedef math::Size2_<T> size_type;

    value_type width;
    value_type height;
    value_type x;
    value_type y;

    Crop2_() : width(0), height(0), x(0), y(0) {}

    Crop2_(value_type width, value_type height
           , value_type x = 0, value_type y = 0)
        : width(width), height(height), x(x), y(y) {};

    Crop2_(const size_type &size, value_type x = 0, value_type y = 0)
        : width(size.width), height(size.height), x(x), y(y) {};

    size_type size() const { return size_type(width, height); }

    template <typename U>
    explicit Crop2_(const Crop2_<U> &v)
        : width(v.width), height(v.height), x(v.x), y(v.y)
    {}
};

typedef Crop2_<int> Crop2i;
typedef Crop2_<double> Crop2f;
typedef Crop2i Crop2;

/** Scale crop by given scale in x- and y-direction)
 */
template<typename T>
Crop2_<double> scale(const Crop2_<T> &crop
                     , const math::Point2_<double> &scale);


template<typename CharT, typename Traits, typename T>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Crop2_<T> &v);

template<typename CharT, typename Traits, typename T>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Crop2_<T> &v);

// impelemtation

template<typename T>
inline Crop2_<double> scale(const Crop2_<T> &crop
                            , const math::Point2_<double> &scale)
{
    return Crop2_<double>(crop.width * scale(0)
                          , crop.height * scale(1)
                          , scale(0) * (0.5 + crop.x)
                          , scale(1) * (0.5 + crop.y));
}

template<typename CharT, typename Traits, typename T>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Crop2_<T> &v)
{
    std::ios::fmtflags flags(os.flags());
    os << v.width << "x" << v.height << std::showpos << v.x << v.y;
    os.flags(flags);
    return os;
}

#if !(defined(_MSC_VER) && defined(__CUDACC__))
template<typename CharT, typename Traits, typename T>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Crop2_<T> &v)
{
    using boost::spirit::qi::auto_;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::omit;
    using boost::spirit::qi::match;

    char sign1, sign2;

    is >> match((auto_ >> omit['x'] >> auto_
                 >> (char_('+') | char_('-')) >> auto_
                 >> (char_('+') | char_('-')) >> auto_)
                , v.width, v.height, sign1, v.x, sign2, v.y);

    if (sign1 == '-') { v.x = -v.x; }
    if (sign2 == '-') { v.y = -v.y; }
    return is;
}
#endif

} // namespace imgproc

#endif // imgproc_crop_included_hpp_
