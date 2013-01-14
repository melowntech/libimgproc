/**
 * @file georeferencing.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Georeferencing.
 */

#ifndef imgproc_georeferencing_hpp_included_
#define imgproc_georeferencing_hpp_included_

#include <cstddef>

#include "math/geometry_core.hpp"

namespace imgproc {

template <typename T>
struct Georeferencing2_;

typedef Georeferencing2_<int> Georeferencing2i;
typedef Georeferencing2_<double> Georeferencing2f;
typedef Georeferencing2f Georeferencing2;

template <typename T>
math::Extents2_<T> extents(const Georeferencing2_<T> &ge);

template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const Georeferencing2_<T> &e);

template<typename CharT, typename Traits, typename T>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is
           , Georeferencing2_<T> &e);

template <typename T>
struct Georeferencing3_;

typedef Georeferencing3_<int> Georeferencing3i;
typedef Georeferencing3_<double> Georeferencing3f;
typedef Georeferencing3f Georeferencing3;

template <typename T>
math::Extents3_<T> extents(const Georeferencing3_<T> &ge);

template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &is
           , const Georeferencing3_<T> &e);

template<typename CharT, typename Traits, typename T>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &os
           , Georeferencing3_<T> &e);

// implementation

namespace detail {
    template <typename T>
    struct GeoreferencingIterator;
}

template <typename T>
struct Georeferencing2_
{
    typedef T value_type;
    typedef math::Point2_<T> point_type;

    point_type ul;
    point_type ur;
    point_type lr;
    point_type ll;

    Georeferencing2_() : ul(), ur(), lr(), ll() {}

    explicit Georeferencing2_(const point_type &p)
        : ul(p), ur(p), lr(p), ll(p)
    {}

    Georeferencing2_(const point_type &ul, const point_type &ur
              , const point_type &lr, const point_type &ll)
        : ul(ul), ur(ur), lr(lr), ll(ll)
    {}

    template <typename U>
    explicit Georeferencing2_(const Georeferencing2_<U> &e)
        : ul(e.ul), ur(e.ur), lr(e.lr), ll(e.ll)
    {}

    typedef detail::GeoreferencingIterator<Georeferencing2_<T>> iterator;

    iterator begin();
    iterator end();
};

template <typename T>
inline math::Extents2_<T> extents(const Georeferencing2_<T> &ge)
{
    math::Extents2_<T> e(ge.ul);
    update(e, ge.ur);
    update(e, ge.lr);
    update(e, ge.ll);
    return e;
}

template <typename T>
struct Georeferencing3_
{
    typedef T value_type;
    typedef math::Point3_<T> point_type;

    point_type ul;
    point_type ur;
    point_type lr;
    point_type ll;

    Georeferencing3_() : ul(), ur(), lr(), ll() {}

    explicit Georeferencing3_(const point_type &p)
        : ul(p), ur(p), lr(p), ll(p)
    {}

    Georeferencing3_(const point_type &ul, const point_type &ur
              , const point_type &lr, const point_type &ll)
        : ul(ul), ur(ur), lr(lr), ll(ll)
    {}

    template <typename U>
    explicit Georeferencing3_(const Georeferencing3_<U> &e)
        : ul(e.ul), ur(e.ur), lr(e.lr), ll(e.ll)
    {}

    typedef detail::GeoreferencingIterator<Georeferencing3_<T>> iterator;

    iterator begin();
    iterator end();
};

template <typename T>
inline math::Extents3_<T> extents(const Georeferencing3_<T> &ge)
{
    math::Extents3_<T> e(ge.ul);
    update(e, ge.ur);
    update(e, ge.lr);
    update(e, ge.ll);
    return e;
}

template <typename T>
struct detail::GeoreferencingIterator {
    typedef typename T::point_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef std::random_access_iterator_tag iterator_category;

    GeoreferencingIterator() : gr_(), index_(4) {}

    GeoreferencingIterator(T *gr) : gr_(gr), index_() {}

    value_type& operator*() const {
        switch (index_) {
        case 0: return gr_->ul;
        case 1: return gr_->ur;
        case 2: return gr_->lr;
        default: return gr_->ll;
        }
    }

    value_type* operator->() const {
        switch (index_) {
        case 0: return &gr_->ul;
        case 1: return &gr_->ur;
        case 2: return &gr_->lr;
        default: return &gr_->ll;
        }
    }

    GeoreferencingIterator operator++() {
        ++index_;
        return *this;
    }

    GeoreferencingIterator operator++(int) {
        auto i(*this);
        ++index_;
        return i;
    }

    GeoreferencingIterator operator--() {
        --index_;
        return *this;
    }

    GeoreferencingIterator operator--(int) {
        auto i(*this);
        --index_;
        return i;
    }

    bool operator==(const GeoreferencingIterator &o) {
        return (gr_ == o.gr_) && (index_ == o.index_);
    }

    bool operator!=(const GeoreferencingIterator &o) {
        return !operator==(o);
    }

private:
    T *gr_;
    unsigned int index_;
};

template <typename T>
inline typename Georeferencing2_<T>::iterator Georeferencing2_<T>::begin()
{
    return Georeferencing2_<T>::iterator(this);
}

template <typename T>
inline typename Georeferencing2_<T>::iterator Georeferencing2_<T>::end()
{
    return Georeferencing2_<T>::iterator();
}

template <typename T>
inline typename Georeferencing3_<T>::iterator Georeferencing3_<T>::begin()
{
    return Georeferencing3_<T>::iterator(this);
}

template <typename T>
inline typename Georeferencing3_<T>::iterator Georeferencing3_<T>::end()
{
    return Georeferencing3_<T>::iterator();
}

template<typename CharT, typename Traits, typename T>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Georeferencing2_<T> &e)
{
    return os << e.ul << ',' << e.ur << ',' << e.lr << ',' << e.ll;
}

template<typename CharT, typename Traits, typename T>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Georeferencing2_<T> &e)
{
    (void) e;
    return is;
}

template<typename CharT, typename Traits, typename T>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Georeferencing3_<T> &e)
{
    return os << e.ul << ',' << e.ur << ',' << e.lr << ',' << e.ll;
}

template<typename CharT, typename Traits, typename T>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Georeferencing3_<T> &e)
{
    (void) e;
    return is;
}

} // namespace imgproc

#endif // imgproc_georeferencing_hpp_included_
