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

namespace imgproc { namespace exif {

typedef boost::rational<long> Rational;

const Rational inch(254, 10000);
const Rational centimeter(1, 100);

#define DECLARE_EXCEPTION(type, base) \
    struct type : public base { type(const std::string &msg) : base(msg) {} }

DECLARE_EXCEPTION(Error, std::runtime_error);
DECLARE_EXCEPTION(NoSuchTag, Error);
DECLARE_EXCEPTION(NoConversionAvailable, Error);
DECLARE_EXCEPTION(InvalidValue, Error);

#undef DECLARE_EXCEPTION

class Exif {
public:
    Exif(const boost::filesystem::path &path)
        : path_(path), ed_(open(path))
    {}

    class Entry;

    Entry getEntry(ExifTag tag, ExifIfd ifd = EXIF_IFD_COUNT) const;

    Rational getFPResolutionUnit(ExifIfd ifd = EXIF_IFD_COUNT) const;

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
    const T& data() const { return *reinterpret_cast<const T*>(e_->data); }

    template <typename T> T as() const;

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

namespace detail {

template <typename T, class Enable = void> T convert(const Exif::Entry &e);

template <typename T
          , class = typename std::enable_if<std::is_arithmetic<T>
                                            ::value>::type>
T convert(const Exif::Entry &e)
{
    switch (e.format()) {
    case EXIF_FORMAT_BYTE: return e.data< ::ExifByte>();
    case EXIF_FORMAT_SHORT: return e.data< ::ExifShort>();
    case EXIF_FORMAT_LONG: return e.data< ::ExifLong>();
    case EXIF_FORMAT_SBYTE: return e.data< ::ExifSByte>();
    case EXIF_FORMAT_SSHORT: return e.data< ::ExifSShort>();
    case EXIF_FORMAT_SLONG: return e.data< ::ExifSLong>();
    case EXIF_FORMAT_FLOAT: return e.data<float>();
    case EXIF_FORMAT_DOUBLE: return e.data<double>();

    case EXIF_FORMAT_RATIONAL: {
        const auto &v(e.data< ::ExifRational>());
        return T(v.numerator) / T(v.denominator);
    }

    case EXIF_FORMAT_SRATIONAL: {
        const auto &v(e.data< ::ExifSRational>());
        return T(v.numerator) / T(v.denominator);
    }
    default:
        LOGTHROW(err1, NoConversionAvailable)
            << "Cannot convert from " << e.formatName() << " numeric type.";
    }
    throw;
}

template <>
inline std::string convert<std::string, void>(const Exif::Entry &e)
{
    return boost::lexical_cast<std::string>(e);
}

template <>
inline Rational convert<Rational, void>(const Exif::Entry &e)
{
    switch (e.format()) {
    case EXIF_FORMAT_BYTE: return { e.data< ::ExifByte>() };
    case EXIF_FORMAT_SHORT: return { e.data< ::ExifShort>() };
    case EXIF_FORMAT_LONG: return { e.data< ::ExifLong>() };
    case EXIF_FORMAT_SBYTE: return { e.data< ::ExifSByte>() };
    case EXIF_FORMAT_SSHORT: return { e.data< ::ExifSShort>() };
    case EXIF_FORMAT_SLONG: return { e.data< ::ExifSLong>() };

    case EXIF_FORMAT_RATIONAL: {
        const auto &v(e.data< ::ExifRational>());
        return { v.numerator, v.denominator };
    }

    case EXIF_FORMAT_SRATIONAL: {
        const auto &v(e.data< ::ExifSRational>());
        return { v.numerator, v.denominator };
    }
    default:
        LOGTHROW(err1, NoConversionAvailable)
            << "Cannot convert from " << e.formatName() << " to rational.";
    }
    throw;
}

} // namespace detail

template <typename T> inline T Exif::Entry::as() const
{
    return detail::convert<T>(*this);
}

} } // namespace imgproc::exif

#endif // imgproc_exif_hpp_included_
