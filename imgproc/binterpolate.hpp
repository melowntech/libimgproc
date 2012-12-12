/**
 * @file binterpolate.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Bilinear interpolation
 */

#ifndef imgproc_binterpolate_hpp_included_
#define imgproc_binterpolate_hpp_included_

#include <cmath>
#include <math/geometry_core.hpp>

namespace imgproc {

namespace gil = boost::gil;

namespace detail {
    inline int clamp(int value, int max) {
        if (value < 0) { return 0; }
        if (value > max) { return max; }
        return value;
    }

    inline math::Point2 fitIn01(math::Point2 p) {
        if (p(0) < 0.) {
            p(0) = 0.;
        } else if (p(0) > 1.) {
            p(0) = 1.;
        }

        if (p(1) < 0.) {
            p(1) = 0.;
        } else if (p(1) > 1.) {
            p(1) = 1.;
        }
        return p;
    }
} // namespace detail


/** NB: Adds interpolated values to result. Set result to zero prior calling
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
            int u(detail::clamp(floor(0), width - 1) + m);
            int v(detail::clamp(floor(0), height - 1) + n);

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
