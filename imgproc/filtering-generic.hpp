/**
 * @file filtering-tmp.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Generic image filtering
 */

#ifndef imgproc_filtering_generic_hpp_included_
#define imgproc_filtering_generic_hpp_included_

#include "./filtering.hpp"

namespace imgproc {
/**
 * Reconstruction is a class that allows reconstruct() call to access input
 * raster, check pixel validity, satureate output and handle exceptional cases.
 *
 * It models the following concepts:
 *
 * class ReconstructionConcept {
 * public:
 *      // underlying data types
 *      typedef unspecified_type value_type;
 *      typedef unspecified_type channel_type;
 *
 *      // number of channels
 *      int channels() const;
 *
 *      // returns value at given pixel coordinates
 *      const value_type& operator()(int x, int y) const;
 *
 *      // returns whether there is a valid pixel at given coordinates bool
 *      valid(int x, int y) const;
 *
 *      // Converts computed value to channel_type and saturates it if it is
 *      //  out of bound
 *      channel_type saturate(double value) const;
 *
 *      // Handles undefined value; can throw an UndefinedValueError exception
 *      channel_type undefined() const
 * };
 */

struct UndefinedValueError : std::runtime_error {
    UndefinedValueError(const std::string &msg) : std::runtime_error(msg) {}
};

template<typename Reconstruction, typename Filter2>
typename Reconstruction::value_type
reconstruct(const Reconstruction &r, const Filter2 &filter
            , const math::Point2 &pos)
{
    // calculate filtering window
    const int x1(std::floor(pos(0) - filter.halfwinx()));
    const int x2(std::ceil (pos(0) + filter.halfwinx()));
    const int y1(std::floor(pos(1) - filter.halfwiny()));
    const int y2(std::ceil (pos(1) + filter.halfwiny()));

    const int numChannels(r.channels());

    // accumulate values for whole filtering window
    double weightSum(0), valueSum[10] = { 0.0 };
    for (int i = y1; i <= y2; ++i) {
        for (int j = x1; j <= x2; ++j) {
            const double weight(filter(j - pos(0), i - pos(1)));

            if (r.valid(j, i)) {
                const auto &value(r(j, i));
                for (int k = 0; k < numChannels; ++k) {
                    valueSum[k] += weight * value[k];
                }
                weightSum += weight;
            }
        }
    }

    // calculate and return result
    typename Reconstruction::value_type retval;
    for (int i = 0; i < numChannels; ++i) {
        if (weightSum > 1e-15) {
            retval[i] = r.saturate(valueSum[i] / weightSum);
        } else {
            retval[i] = r.undefined();
        }
    }
    return retval;
}

/** Reconstruction plugin to add raster mask support.
 */
class MaskedPlugin {
public:
    MaskedPlugin(const quadtree::RasterMask &mask) : mask_(mask) {}
    bool valid(int x, int y) const { return mask_.get(x, y); }
private:
    const quadtree::RasterMask &mask_;
};

// OpenCV Matrix support

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
class MatReconstruction {
public:
    typedef detail::CvPixelTraits<ValueType> Traits;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::channel_type channel_type;

    MatReconstruction(const cv::Mat &input) : input_(input) {}

    int channels() const { return input_.channels(); };

    const value_type& operator()(int x, int y) const {
        return input_.at<value_type>(y, x);
    }

    bool valid(int x, int y) const {
        return (x >= 0) && (x < input_.cols) && (y >= 0) && (y < input_.rows);
    }

    channel_type saturate(double value) const {
        return cv::saturate_cast<channel_type>(value);
    }

    channel_type undefined() const { return {}; }

private:
    const cv::Mat &input_;
};

template <typename ValueType>
class MaskedMatReconstruction
    : public MatReconstruction<ValueType>
    , public MaskedPlugin
{
public:
    MaskedMatReconstruction(const cv::Mat &input
                            , const quadtree::RasterMask &mask)
        : MatReconstruction<ValueType>(input), MaskedPlugin(mask)
    {}

    bool valid(int x, int y) const {
        return (MatReconstruction<ValueType>::valid(x, y)
                && MaskedPlugin::valid(x, y));
    }
};

// GIL view support

template <typename ViewType>
class GilViewReconstruction
    : private detail::PixelLimits<typename ViewType::value_type>
{
public:
    typedef typename ViewType::value_type value_type;
    typedef typename gil::channel_type<value_type>::type
        channel_type;

    GilViewReconstruction(const ViewType &input) : input_(input) {}

    int channels() const { return gil::num_channels<ViewType>::value; };

    const value_type& operator()(int x, int y) const {
        return *input_.xy_at(x, y);
    }

    bool valid(int x, int y) const {
        return ((x >= 0) && (x < input_.width())
                && (y >= 0) && (y < input_.height()));
    }

    channel_type saturate(double value) const {
        return detail::PixelLimits<typename ViewType::value_type>
            ::clamp(value);
    }

    channel_type undefined() const { return {}; }

private:
    const ViewType &input_;
};

template <typename ViewType>
class MaskedGilViewReconstruction
    : public GilViewReconstruction<ViewType>
    , public MaskedPlugin
{
public:
    MaskedGilViewReconstruction(const ViewType &input
                                , const quadtree::RasterMask &mask)
        : MaskedGilViewReconstruction<ViewType>(input), MaskedPlugin(mask)
    {}

    bool valid(int x, int y) const {
        return (MaskedGilViewReconstruction<ViewType>::valid(x, y)
                && MaskedPlugin::valid(x, y));
    }
};

template <typename Loc>
GilViewReconstruction<gil::image_view<Loc> >
gilViewReconstruction(const gil::image_view<Loc> &view)
{
    return { view };
}

template <typename Loc>
MaskedGilViewReconstruction<gil::image_view<Loc> >
gilViewReconstruction(const gil::image_view<Loc> &view
                      , const quadtree::RasterMask &mask)
{
    return { view, mask };
}

} // namespace imgproc

#endif // imgproc_filtering_generic_hpp_included_
