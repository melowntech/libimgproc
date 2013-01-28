/**
 * @file georeferencing.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Georeferencing.
 */

#ifndef imgproc_georeferencing_hpp_included_
#define imgproc_georeferencing_hpp_included_

#include <cstddef>
#include <iostream>

#include "math/geometry_core.hpp"
#include "utility/streams.hpp"

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
    template <typename T> struct GeoreferencingIterator;
    template <typename T> struct ConstGeoreferencingIterator;
} // namespace detail

template <typename T>
struct Georeferencing2_
{
    typedef math::Point2_<T> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;

    value_type ul;
    value_type ur;
    value_type lr;
    value_type ll;

    Georeferencing2_() : ul(), ur(), lr(), ll() {}

    explicit Georeferencing2_(const value_type &p)
        : ul(p), ur(p), lr(p), ll(p)
    {}

    Georeferencing2_(const value_type &ul, const value_type &ur
              , const value_type &lr, const value_type &ll)
        : ul(ul), ur(ur), lr(lr), ll(ll)
    {}

    template <typename U>
    explicit Georeferencing2_(const Georeferencing2_<U> &e)
        : ul(e.ul), ur(e.ur), lr(e.lr), ll(e.ll)
    {}

    reference operator[](int index);

    const_reference operator[](int index) const;

    typedef detail::GeoreferencingIterator<Georeferencing2_<T>> iterator;

    typedef detail::ConstGeoreferencingIterator<Georeferencing2_<T>>
        const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin();
    iterator end();
    const_iterator end() const;
    const_iterator cend();
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
    typedef math::Point3_<T> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;

    value_type ul;
    value_type ur;
    value_type lr;
    value_type ll;

    Georeferencing3_() : ul(), ur(), lr(), ll() {}

    explicit Georeferencing3_(const value_type &p)
        : ul(p), ur(p), lr(p), ll(p)
    {}

    Georeferencing3_(const value_type &ul, const value_type &ur
              , const value_type &lr, const value_type &ll)
        : ul(ul), ur(ur), lr(lr), ll(ll)
    {}

    template <typename U>
    explicit Georeferencing3_(const Georeferencing3_<U> &e)
        : ul(e.ul), ur(e.ur), lr(e.lr), ll(e.ll)
    {}

    reference operator[](int index);

    const_reference operator[](int index) const;

    typedef detail::GeoreferencingIterator<Georeferencing3_<T>> iterator;

    typedef detail::ConstGeoreferencingIterator<Georeferencing3_<T>>
        const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin();
    iterator end();
    const_iterator end() const;
    const_iterator cend();
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
    typedef typename T::value_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename T::pointer pointer;
    typedef typename T::reference reference;
    typedef std::random_access_iterator_tag iterator_category;

    GeoreferencingIterator(T *gr, int index = 0) : gr_(gr), index_(index) {}

    value_type& operator*() const {
        return (*gr_)[index_];
    }

    value_type* operator->() const {
        return (*gr_)[index_];
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
    int index_;
};

template <typename T>
struct detail::ConstGeoreferencingIterator {
    typedef typename T::value_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename T::pointer pointer;
    typedef typename T::const_reference reference;
    typedef std::random_access_iterator_tag iterator_category;

    ConstGeoreferencingIterator(const T *gr, int index = 0) : gr_(gr), index_(index) {}

    const value_type& operator*() const {
        return (*gr_)[index_];
    }

    const value_type* operator->() const {
        return (*gr_)[index_];
    }

    ConstGeoreferencingIterator operator++() {
        ++index_;
        return *this;
    }

    ConstGeoreferencingIterator operator++(int) {
        auto i(*this);
        ++index_;
        return i;
    }

    ConstGeoreferencingIterator operator--() {
        --index_;
        return *this;
    }

    ConstGeoreferencingIterator operator--(int) {
        auto i(*this);
        --index_;
        return i;
    }

    bool operator==(const ConstGeoreferencingIterator &o) {
        return (gr_ == o.gr_) && (index_ == o.index_);
    }

    bool operator!=(const ConstGeoreferencingIterator &o) {
        return !operator==(o);
    }

private:
    const T *gr_;
    int index_;
};

template <typename T>
inline typename Georeferencing2_<T>::reference
Georeferencing2_<T>::operator[](int index)
{
    switch (index) {
    case 0: return ul;
    case 1: return ur;
    case 2: return lr;
    default: return ll;
    }
}

template <typename T>
inline typename Georeferencing2_<T>::const_reference
Georeferencing2_<T>::operator[](int index) const
{
    switch (index) {
    case 0: return ul;
    case 1: return ur;
    case 2: return lr;
    default: return ll;
    }
}

template <typename T>
inline typename Georeferencing3_<T>::reference
Georeferencing3_<T>::operator[](int index)
{
    switch (index) {
    case 0: return ul;
    case 1: return ur;
    case 2: return lr;
    default: return ll;
    }
}

template <typename T>
inline typename Georeferencing3_<T>::const_reference
Georeferencing3_<T>::operator[](int index) const
{
    switch (index) {
    case 0: return ul;
    case 1: return ur;
    case 2: return lr;
    default: return ll;
    }
}

template <typename T>
inline typename Georeferencing2_<T>::iterator Georeferencing2_<T>::begin()
{
    return Georeferencing2_<T>::iterator(this);
}

template <typename T>
inline typename
Georeferencing2_<T>::const_iterator Georeferencing2_<T>::begin() const
{
    return Georeferencing2_<T>::const_iterator(this);
}

template <typename T>
inline typename
Georeferencing2_<T>::const_iterator Georeferencing2_<T>::cbegin()
{
    return Georeferencing2_<T>::const_iterator(this);
}

template <typename T>
inline typename Georeferencing2_<T>::iterator Georeferencing2_<T>::end()
{
    return Georeferencing2_<T>::iterator(this, 4);
}

template <typename T>
inline typename
Georeferencing2_<T>::const_iterator Georeferencing2_<T>::end() const
{
    return Georeferencing2_<T>::const_iterator(this, 4);
}

template <typename T>
inline typename
Georeferencing2_<T>::const_iterator Georeferencing2_<T>::cend()
{
    return Georeferencing2_<T>::const_iterator(this, 4);
}


template <typename T>
inline typename Georeferencing3_<T>::iterator Georeferencing3_<T>::begin()
{
    return Georeferencing3_<T>::iterator(this);
}

template <typename T>
inline typename
Georeferencing3_<T>::const_iterator Georeferencing3_<T>::begin() const
{
    return Georeferencing3_<T>::const_iterator(this);
}

template <typename T>
inline typename
Georeferencing3_<T>::const_iterator Georeferencing3_<T>::cbegin()
{
    return Georeferencing3_<T>::const_iterator(this);
}

template <typename T>
inline typename Georeferencing3_<T>::iterator Georeferencing3_<T>::end()
{
    return Georeferencing3_<T>::iterator(this, 4);
}

template <typename T>
inline typename
Georeferencing3_<T>::const_iterator Georeferencing3_<T>::end() const
{
    return Georeferencing3_<T>::const_iterator(this, 4);
}

template <typename T>
inline typename
Georeferencing3_<T>::const_iterator Georeferencing3_<T>::cend()
{
    return Georeferencing3_<T>::const_iterator(this, 4);
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
    auto comma(utility::expect(','));
    return is >> e.ul >> comma >> e.ur >> comma >> e.lr >> comma >> e.ll;
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
    auto comma(utility::expect(','));
    return is >> e.ul >> comma >> e.ur >> comma >> e.lr >> comma >> e.ll;
}

} // namespace imgproc

#endif // imgproc_georeferencing_hpp_included_
