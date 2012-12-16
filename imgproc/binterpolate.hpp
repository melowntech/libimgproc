/**
 * @file binterpolate.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Bilinear interpolation
 */

#ifndef imgproc_binterpolate_hpp_included_
#define imgproc_binterpolate_hpp_included_

#include <cmath>
#include <fstream>
#include <math/geometry_core.hpp>


namespace imgproc {

namespace gil = boost::gil;

namespace detail {
    template <typename T, typename Q>
    inline T clamp(T value, Q max) {
        if (value < T(0)) { return T(0); }
        if (value > T(max)) { return T(max); }
        return value;
    }

    inline math::Point2 fitIn01(math::Point2 p) {
        p(0) = clamp(p(0), 1.);
        p(1) = clamp(p(1), 1.);
        return p;
    }
} // namespace detail


template <typename T, typename Q>
bool insideImage(T px, T py, Q width, Q height)
{
    return ((px >= T(0)) && (px <= T(width - 1))
            && (py >= T(0)) && (py <= T(height - 1)));
}

template <typename T, typename Q>
bool insideImage(const math::Point2_<T> &point, const math::Size2_<Q> &size)
{
    return insideImage(point(0), point(1), size.width, size.height);
}

template <typename T, typename Q>
bool insideImage(const math::Point2_<T> &point, Q width, Q height)
{
    return insideImage(point(0), point(1), width, height);
}

/** NB: Adds interpolated values to the result. Set result to zero prior calling
 *  this function if this behaviour is not desired.
 */
template <typename T, typename Q, typename R>
void bilinearInterpolate(const T *data, size_t pitch, size_t width
                         , size_t height, size_t channels
                         , const math::Point2_<Q> &point
                         , R *result, size_t resultChannels = 0)
{
    if (!resultChannels) {
        resultChannels = channels;
    }
    math::Point2i floor(point);
    math::Point2 dpoint(detail::fitIn01(point - floor));

    for (int m(0); m <= 1; ++m) {
        for (int n(0); n <= 1; ++n) {
            int u(detail::clamp(floor(0) + m, width - 1));
            int v(detail::clamp(floor(1) + n, height - 1));

            size_t offset(v * pitch + u * channels);
            double s(fabs(1 - m - dpoint(0)) * fabs(1 - n - dpoint(1)));
            for (size_t c(0); c < resultChannels; ++c) {
                result[c] += s * data[offset + c];
            }
        }
    }
}

/** NB: Adds interpolated values to result. Set result to zero prior calling
 *  this function if this behaviour is not desired.
 */
template <typename T, typename Q, typename R>
void bilinearInterpolate(const T *data, size_t pitch, const math::Size2 &size
                         , size_t channels, const math::Point2_<Q> &point
                         , R *result, size_t resultChannels = 0)
{
    return bilinearInterpolate(data, pitch, size.width, size.height
                               , channels, point, result, resultChannels);
}

} // namespace imgproc

#endif // imgproc_binterpolate_hpp_included_
