/**
 * @file const-raster.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Generic constant raster accessor
 */

#ifndef imgproc_const_raster_hpp_included_
#define imgproc_const_raster_hpp_included_

#include <boost/gil/gil_all.hpp>

#if IMGPROC_HAS_OPENCV
#include <opencv2/core/core.hpp>
#endif // IMGPROC_HAS_OPENCV

#include "math/math.hpp"
#include "math/geometry_core.hpp"
#include "./rastermask/quadtree.hpp"

namespace imgproc {
namespace gil = boost::gil;

/**
 * Const Raster is a class that allows generalized read-only access to
 * underlying raster.
 *
 * It models the following concepts:
 *
 * class ConstRasterConcept {
 * public:
 *      // underlying data types
 *      typedef unspecified_type value_type;
 *      typedef unspecified_type channel_type;
 *
 *      // number of channels
 *      int channels() const;
 *
 *      // width, height and size of an image
 *      int width() const;
 *      int height() const;
 *      math::Size2i size() const;
 *
 *      // returns value at given pixel coordinates
 *      const value_type& operator()(int x, int y) const;
 *
 *      // returns whether there is a valid pixel at given coordinates bool
 *      valid(int x, int y) const;
 *
 *      // Converts computed value to channel_type and saturates it if it is
 *      // out of bound
 *      channel_type saturate(double value) const;
 *
 *      // Handles undefined value; can throw an UndefinedValueError exception
 *      // Works with whole value (i.e. pixel) at once.
 *      value_type undefined() const
 * };
 */

struct UndefinedValueError : std::runtime_error {
    UndefinedValueError(const std::string &msg) : std::runtime_error(msg) {}
};

template<typename ConstRaster, typename Filter2>
typename ConstRaster::value_type
reconstruct(const ConstRaster &r, const Filter2 &filter
            , const math::Point2 &pos);

/** CRTP base for pixel in-bounds validation
 */
template <typename Derived>
struct BoundsValidator {
    typedef BoundsValidator<Derived> BoundsValidatorType;

    bool valid(int x, int y) const {
        return ((x >= 0)
                && (x < static_cast<const Derived*>(this)->width())
                && (y >= 0)
                && (y < static_cast<const Derived*>(this)->height()));
    }
};

/** ConstRaster plugin to add raster mask support.
 */
class MaskedPlugin {
public:
    MaskedPlugin(const quadtree::RasterMask &mask) : mask_(mask) {}
    bool valid(int x, int y) const { return mask_.get(x, y); }
private:
    const quadtree::RasterMask &mask_;
};

/************************************************************************
 * OpenCV Matrix support
 * Available only if compiled with OpenCV
 ************************************************************************/
#if IMGPROC_HAS_OPENCV
namespace detail {
    template <typename ValueType> struct CvPixelTraits {
        typedef cv::Vec<ValueType, 1> value_type;
        typedef ValueType channel_type;
    };

    template <>
    template <typename T, int N>
    struct CvPixelTraits<cv::Vec<T, N> > {
        typedef cv::Vec<T, N> value_type;
        typedef T channel_type;
    };
} // namespace detail

template <typename ValueType>
class CvConstRaster
    : public BoundsValidator<CvConstRaster<ValueType> >
{
public:
    typedef detail::CvPixelTraits<ValueType> Traits;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::channel_type channel_type;

    explicit CvConstRaster(const cv::Mat &mat) : mat_(mat) {}

    int channels() const { return mat_.channels(); };

    const value_type& operator()(int x, int y) const {
        return mat_.at<value_type>(y, x);
    }

    channel_type saturate(double value) const {
        return cv::saturate_cast<channel_type>(value);
    }

    value_type undefined() const { return {}; }

    int width() const { return mat_.cols; }
    int height() const { return mat_.rows; }
    math::Size2i size() const { return { mat_.cols, mat_.rows }; }

private:
    const cv::Mat &mat_;
};

template <typename ValueType>
class MaskedCvConstRaster
    : public MaskedPlugin
    , public CvConstRaster<ValueType>
{
public:
    MaskedCvConstRaster(const cv::Mat &mat
                        , const quadtree::RasterMask &mask)
        : MaskedPlugin(mask), CvConstRaster<ValueType>(mat)
    {}

    bool valid(int x, int y) const {
        return (CvConstRaster<ValueType>::valid(x, y)
                && MaskedPlugin::valid(x, y));
    }
};

template <typename ValueType>
CvConstRaster<ValueType>
cvConstRaster(const cv::Mat &mat)
{
    return CvConstRaster<ValueType>(mat);
}

template <typename ValueType>
CvConstRaster<ValueType>
cvConstRaster(const cv::Mat &mat, const quadtree::RasterMask &mask)
{
    return { mat, mask };
}
#endif // IMGPROC_HAS_OPENCV

namespace detail {

template <typename PixelType>
class ValueLimits {
public:
    typedef typename gil::channel_type<PixelType>::type channel_type;

    ValueLimits()
        : min_(gil::channel_traits<channel_type>::min_value())
        , max_(gil::channel_traits<channel_type>::max_value())
    {}

    channel_type clamp(double value) const {
        if (value < min_) { return min_; }
        else if (value > max_) { return max_; }
        return value;
    }

private:
    channel_type min_, max_;
};

template <>
class ValueLimits<gil::pixel<float, gil::gray_layout_t> > {
public:
    typedef gil::pixel<float, gil::gray_layout_t> pixel_type;
    ValueLimits() {}

    double clamp(double value) const { return value; }
};

} // namespace detail

/************************************************************************
 * GIL view support
 ************************************************************************/
template <typename ViewType>
class GilConstRaster
    : private detail::ValueLimits<typename ViewType::value_type>
    , public BoundsValidator<GilConstRaster<ViewType> >
{
public:
    typedef typename ViewType::value_type value_type;
    typedef typename gil::channel_type<value_type>::type
        channel_type;

    explicit GilConstRaster(const ViewType &view) : view_(view) {}

    int channels() const { return gil::num_channels<ViewType>::value; };

    const value_type& operator()(int x, int y) const {
        return *view_.xy_at(x, y);
    }

    channel_type saturate(double value) const {
        return detail::ValueLimits<typename ViewType::value_type>
            ::clamp(value);
    }

    value_type undefined() const { return {}; }

    int width() const { return view_.width(); }
    int height() const { return view_.height(); }
    math::Size2i size() const { return { view_.width(), view_.height() }; }

private:
    const ViewType &view_;
};

template <typename ViewType>
class MaskedGilConstRaster
    : public MaskedPlugin
    , public GilConstRaster<ViewType>
{
public:
    MaskedGilConstRaster(const ViewType &view
                         , const quadtree::RasterMask &mask)
        : MaskedPlugin(mask), GilConstRaster<ViewType>(view)
    {}

    bool valid(int x, int y) const {
        return (MaskedGilConstRaster<ViewType>::valid(x, y)
                && MaskedPlugin::valid(x, y));
    }
};

template <typename Loc>
GilConstRaster<gil::image_view<Loc> >
gilConstRaster(const gil::image_view<Loc> &view)
{
    return GilConstRaster<gil::image_view<Loc> >(view);
}

template <typename Loc>
MaskedGilConstRaster<gil::image_view<Loc> >
gilConstRaster(const gil::image_view<Loc> &view
          , const quadtree::RasterMask &mask)
{
    return { view, mask };
}

} // namespace imgproc

#endif // imgproc_const_raster_hpp_included_
