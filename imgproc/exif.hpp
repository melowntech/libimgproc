#ifndef imgproc_exif_hpp_included_
#define imgproc_exif_hpp_included_

#include <stdexcept>
#include <iostream>
#include <type_traits>

#include <boost/lexical_cast.hpp>
#include <boost/rational.hpp>

#include <boost/filesystem.hpp>

#include <libexif/exif-data.h>

#include "dbglog/dbglog.hpp"

#include "error.hpp"

namespace imgproc { namespace exif {

typedef boost::rational<long> Rational;

const Rational inch(254, 10000);
const Rational centimeter(1, 100);
const Rational millimeter(1, 1000);

#define DECLARE_EXCEPTION(type, base) \
    struct type : public base { type(const std::string &msg) : base(msg) {} }

DECLARE_EXCEPTION(Error, imgproc::Error);
DECLARE_EXCEPTION(NoSuchTag, Error);
DECLARE_EXCEPTION(NoConversionAvailable, Error);
DECLARE_EXCEPTION(InvalidValue, Error);

#undef DECLARE_EXCEPTION

enum class Orientation {
    top_left
    , top_right
    , bottom_right
    , bottom_left
    , left_top
    , right_top
    , right_bottom
    , left_bottom
};

class Exif {
public:
    Exif(const boost::filesystem::path &path)
        : path_(path), ed_(open(path))
    {}

    class Entry;

    // throws if tag not found
    Entry getEntry(ExifTag tag, ExifIfd ifd = EXIF_IFD_COUNT) const;

    // returns tag or default if tag not present
    template <typename T>
    T getEntrySafe(const T& df, ExifTag tag, ExifIfd ifd = EXIF_IFD_COUNT) const;

    Rational getFPResolutionUnit(ExifIfd ifd = EXIF_IFD_COUNT) const;

    Orientation getOrientation(ExifIfd ifd = EXIF_IFD_COUNT) const;

private:
    friend class Entry;

    typedef std::shared_ptr< ::ExifData> Handle;

    Handle open(const boost::filesystem::path &path);

    boost::filesystem::path path_;
    Handle ed_;
};

class Exif::Entry {
public:
    std::string str() const;

    std::size_t size() const { return e_->size; }

    ::ExifFormat format() const { return e_->format; }

    const char* formatName() const { return exif_format_get_name(e_->format); }

    template <typename T>
    const T& data(int idx = 0) const {
        return *( reinterpret_cast<const T*>(e_->data) + idx );
    }

    template <typename T> T as(int idx = 0) const;

private:
    friend class Exif;

    Entry(const Exif::Handle &ed, ::ExifEntry *e) : ed_(ed), e_(e) {}

    Exif::Handle ed_;
    ::ExifEntry *e_;
};

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Exif::Entry &e);

// inline functions implementation

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Exif::Entry &e)
{
    switch (e.format()) {
    case EXIF_FORMAT_BYTE: return os << e.data< ::ExifByte>();
    case EXIF_FORMAT_SHORT: return os << e.data< ::ExifShort>();
    case EXIF_FORMAT_LONG: return os << e.data< ::ExifLong>();
    case EXIF_FORMAT_SBYTE: return os << e.data< ::ExifSByte>();
    case EXIF_FORMAT_UNDEFINED: return os << e.data< ::ExifUndefined>();
    case EXIF_FORMAT_SSHORT: return os << e.data< ::ExifSShort>();
    case EXIF_FORMAT_SLONG: return os << e.data< ::ExifSLong>();
    case EXIF_FORMAT_FLOAT: return os << e.data<float>();
    case EXIF_FORMAT_DOUBLE: return os << e.data<double >();

    case EXIF_FORMAT_RATIONAL: {
        const auto &v(e.data< ::ExifRational>());
        return os << v.numerator << "/" << v.denominator;
    }

    case EXIF_FORMAT_SRATIONAL: {
        const auto &v(e.data< ::ExifSRational>());
        return os << v.numerator << "/" << v.denominator;
    }

    case EXIF_FORMAT_ASCII: {
        const char *ptr();
        os.write(&e.data<char>(), e.size());
    }

    }

    return os;
}

inline std::string Exif::Entry::str() const
{
    return boost::lexical_cast<std::string>(*this);
}

template <typename E, typename T>
inline std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E, T> &os, const Orientation &o)
{
    switch (o) {
    case Orientation::top_left: return os << "top-left";
    case Orientation::top_right: return os << "top-right";
    case Orientation::bottom_right: return os << "bottom-right";
    case Orientation::bottom_left: return os << "bottom-left";
    case Orientation::left_top: return os << "left-top";
    case Orientation::right_top: return os << "right-top";
    case Orientation::right_bottom: return os << "right-bottom";
    case Orientation::left_bottom: return os << "left-bottom";
    }

    os.setstate(std::ios::failbit);
    return os;
}

template <typename E, typename T>
inline std::basic_istream<E, T>&
operator>>(std::basic_istream<E, T> &is, Orientation &o)
{
    std::string s;
    is >> s;

    if (s == "top-left") {
        o = Orientation::top_left;
    } else if (s == "top-right") {
        o = Orientation::top_right;
    } else if (s == "bottom-right") {
        o = Orientation::bottom_right;
    } else if (s == "bottom-left") {
        o = Orientation::bottom_left;
    } else if (s == "left-top") {
        o = Orientation::left_top;
    } else if (s == "right-top") {
        o = Orientation::right_top;
    } else if (s == "right-bottom") {
        o = Orientation::right_bottom;
    } else if (s == "left-bottom") {
        o = Orientation::left_bottom;
    } else {
        o = {}; // because of boost::lexical_cast bug
        is.setstate(std::ios::failbit);
    }

    return is;
}

namespace detail {

template <typename T, class Enable = void> T convert(const Exif::Entry &e, int idx);

template <typename T
          , class = typename std::enable_if<std::is_arithmetic<T>
                                            ::value>::type>
T convert(const Exif::Entry &e, int idx)
{
    switch (e.format()) {
    case EXIF_FORMAT_BYTE: return e.data< ::ExifByte>(idx);
    case EXIF_FORMAT_SHORT: return e.data< ::ExifShort>(idx);
    case EXIF_FORMAT_LONG: return e.data< ::ExifLong>(idx);
    case EXIF_FORMAT_SBYTE: return e.data< ::ExifSByte>(idx);
    case EXIF_FORMAT_SSHORT: return e.data< ::ExifSShort>(idx);
    case EXIF_FORMAT_SLONG: return e.data< ::ExifSLong>(idx);
    case EXIF_FORMAT_FLOAT: return e.data<float>(idx);
    case EXIF_FORMAT_DOUBLE: return e.data<double>(idx);

    case EXIF_FORMAT_RATIONAL: {
        const auto &v(e.data< ::ExifRational>(idx));
        return T(v.numerator) / T(v.denominator);
    }

    case EXIF_FORMAT_SRATIONAL: {
        const auto &v(e.data< ::ExifSRational>(idx));
        return T(v.numerator) / T(v.denominator);
    }
    default:
        LOGTHROW(err1, NoConversionAvailable)
            << "Cannot convert from " << e.formatName() << " numeric type.";
    }
    throw;
}

/**
 * FIXME, idx not working currently, acts as idx = 0 all the time
 */
template <>
inline std::string convert<std::string, void>(const Exif::Entry &e, int)
{
    return boost::lexical_cast<std::string>(e);
}

template <>
inline Rational convert<Rational, void>(const Exif::Entry &e, int idx)
{
    switch (e.format()) {
    case EXIF_FORMAT_BYTE: return { e.data< ::ExifByte>(idx) };
    case EXIF_FORMAT_SHORT: return { e.data< ::ExifShort>(idx) };
    case EXIF_FORMAT_LONG: return { e.data< ::ExifLong>(idx) };
    case EXIF_FORMAT_SBYTE: return { e.data< ::ExifSByte>(idx) };
    case EXIF_FORMAT_SSHORT: return { e.data< ::ExifSShort>(idx) };
    case EXIF_FORMAT_SLONG: return { e.data< ::ExifSLong>(idx) };

    case EXIF_FORMAT_RATIONAL: {
        const auto &v(e.data< ::ExifRational>(idx));
        return { v.numerator, v.denominator };
    }

    case EXIF_FORMAT_SRATIONAL: {
        const auto &v(e.data< ::ExifSRational>(idx));
        return { v.numerator, v.denominator };
    }
    default:
        LOGTHROW(err1, NoConversionAvailable)
            << "Cannot convert from " << e.formatName() << " to rational.";
    }
    throw;
}

} // namespace detail

template <typename T> inline T Exif::Entry::as(int idx) const
{
    return detail::convert<T>(*this, idx);
}

template <typename T>
T Exif::getEntrySafe(const T& df, ExifTag tag, ExifIfd ifd) const
{
    try {
        Entry tmp(getEntry( tag, ifd ));
        return tmp.as<T>();
    } catch (const NoSuchTag &e) {
        LOG(warn1) <<  e.what() << "; Setting tag to default " << df;
    }
    return df;
}

} } // namespace imgproc::exif

#endif // imgproc_exif_hpp_included_
