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

#ifndef imgproc_svg_hpp_included_
#define imgproc_svg_hpp_included_

#include <iostream>

#include "math/geometry_core.hpp"

namespace imgproc { namespace svg {

/** RAII Pair tag support. Emits open tag in ctor, close tag in dtor.
 */
class Tag {
public:
    Tag(std::ostream &os, const std::string &name);

    template<typename AttributeWriter>
    Tag(std::ostream &os, const std::string &name
        , const AttributeWriter &attributeWriter);

    ~Tag();

private:
    std::ostream &os_;
    const std::string name_;
};

struct XmlDeclaration {
    XmlDeclaration(std::ostream &os);
};

struct Svg : XmlDeclaration, Tag {
    Svg(std::ostream &os, const math::Size2 &size);
};

/** RGB color. ColortT must be 3 element array-like type (array, std::array,
 * std::vector, cv::Vec3, ...)
 */
template <typename ColorT>
struct Rgb {
    ColorT value;
    Rgb(const ColorT &value) : value(value) {}
};

/** RGBA color: RGB color with opacity (alpha)
 */
template <typename ColorT>
struct Rgba : Rgb<ColorT> {
    float alpha;
    Rgba(const cv::Vec3b &value, float alpha)
        : Rgb<ColorT>(value), alpha(alpha)
    {}
};

/** Knows how to dump fill (and fill-opacity) style attributes for given color
 *  type.
 */
template <typename ColorT> struct Fill { ColorT color; };

/** Knows how to dump stroke (and stroke-opacity) style attributes for given
 *  color type.
 */
template <typename ColorT> struct Stroke { ColorT color; };

template <typename ColorT>
Rgb<ColorT> color(const ColorT &value) { return { value }; }

template <typename ColorT>
Rgba<ColorT> color(const ColorT &value, float alpha) {
    return { value, alpha };
}

template <typename ColorT>
Rgb<ColorT> color(const Rgb<ColorT> &value) { return value; }

template <typename ColorT>
Rgba<ColorT> color(const Rgba<ColorT> &value) { return value; }

template<typename CharT, typename Traits, typename ColorT>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Rgb<ColorT> &color)
{
    return os << "rgb(" << (unsigned int)(color.value[0])
              << "," << (unsigned int)(color.value[1])
              << "," << (unsigned int)(color.value[2])
              << ")";
}

// fill

template <typename ...Args>
auto fill(Args &&...args)
    -> Fill<decltype(color(std::forward<Args>(args)...))>
{
    return { color(std::forward<Args>(args)...) };
}

template<typename CharT, typename Traits, typename ColorT>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Fill<Rgb<ColorT>> &fill)
{
    return os << "fill:" << fill.color;
}

template<typename CharT, typename Traits, typename ColorT>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Fill<Rgba<ColorT>> &fill)
{
    return os << "fill:" << fill.color
              << ";fill-opacity:" << fill.color.alpha;
}

// stroke

template <typename ...Args>
auto stroke(Args &&...args)
    -> Stroke<decltype(color(std::forward<Args>(args)...))>
{
    return { color(std::forward<Args>(args)...) };
}

template<typename CharT, typename Traits, typename ColorT>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Stroke<Rgb<ColorT>> &stroke)
{
    return os << "stroke:" << stroke.color;
}

template<typename CharT, typename Traits, typename ColorT>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Stroke<Rgba<ColorT>> &stroke)
{
    return os << "stroke:" << stroke.color
              << ";stroke-opacity:" << stroke.color.alpha;
}

inline Tag::Tag(std::ostream &os, const std::string &name)
    : os_(os), name_(name)
{
    os_ << '<' << name << ">\n";
}

template<typename AttributeWriter>
Tag::Tag(std::ostream &os, const std::string &name
    , const AttributeWriter &attributeWriter)
    : os_(os), name_(name)
{
    os_ << '<' << name << ' ';
    attributeWriter(os_);
    os_ << ">\n";
}

inline Tag::~Tag() {
    try { os_ << "</" << name_ << ">\n"; } catch (...) {}
}

inline XmlDeclaration::XmlDeclaration(std::ostream &os) {
    os << "<?xml version=\"1.0\"?>\n";
}

inline Svg::Svg(std::ostream &os, const math::Size2 &size)
    : XmlDeclaration(os)
    , Tag(os, "svg", [size](std::ostream &os)
      {
          os << "xmlns=\"http://www.w3.org/2000/svg\"\n"
             << "     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
             << "     width=\"" << size.width
             << "\" height=\"" << size.height
             << "\"";
      })
{}

} } // namespace imgproc::svg

#endif // imgproc_svg _hpp_included_
