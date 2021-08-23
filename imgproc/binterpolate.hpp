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
 * @file binterpolate.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 * @author Jakub Cerveny <jakub.cerveny@melown.com>
 *
 * Bilinear interpolation
 */

#ifndef imgproc_binterpolate_hpp_included_
#define imgproc_binterpolate_hpp_included_

#include <cmath>
#include <fstream>
#include <math/geometry_core.hpp>


namespace imgproc {

//namespace gil = boost::gil;

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

/** Bilinear interpolation of a color image stored in a cv::Mat.
 *  Example RgbType: cv::Vec<float,3>
 */
template <typename RgbType, typename MatType = uchar>
inline RgbType rgbInterpolate(const cv::Mat &mat, float x, float y)
{
    assert(mat.cols > 1 && mat.rows > 1);
    int nChannels = RgbType::channels;
    assert(nChannels == mat.channels());

    auto getValue([nChannels](const MatType* ptr)
    {
        RgbType res;
        for (int i = 0; i < nChannels; ++i) {
            // copy and convert
            res(i) = ptr[i];
        }

        return res;
    });

    const float eps = 1e-3f;
    float xmax = (float) mat.cols - (1.f + eps);
    float ymax = (float) mat.rows - (1.f + eps);

    if (x < 0.0) { x = 0.0; }
    else if (x > xmax) { x = xmax; }

    if (y < 0.0) { y = 0.0; }
    else if (y > ymax) { y = ymax; }

    int x0 = floor(x);
    int y0 = floor(y);

    float fx = x - x0;
    float fy = y - y0;

    RgbType v00(getValue(mat.ptr<MatType>(y0, x0)));
    RgbType v01(getValue(mat.ptr<MatType>(y0, x0+1)));
    RgbType w0 = v00 + (v01 - v00)*fx;

    RgbType v10(getValue(mat.ptr<MatType>(y0+1, x0)));
    RgbType v11(getValue(mat.ptr<MatType>(y0+1, x0+1)));
    RgbType w1 = v10 + (v11 - v10)*fx;

    return w0 + (w1 - w0)*fy;
}

/** Bilinear interpolation of an image stored in cv::Mat_<InType>.
 *  By default, an uchar grayscale image is interpolated into a float result.
 */
template<typename InType = uchar, typename OutType = float>
OutType interpolate(const cv::Mat &img, float x, float y)
{
    assert(img.cols > 1 && img.rows > 1);

    const float eps = 1e-3f;
    float xmax = (float) img.cols - (1.f + eps);
    float ymax = (float) img.rows - (1.f + eps);

    x = std::min(std::max(x, 0.f), xmax);
    y = std::min(std::max(y, 0.f), ymax);

    int x0 = (int) x; // "floor"
    int y0 = (int) y; // "floor"

    float fx1 = x - x0,  fx0 = 1.f - fx1;
    float fy1 = y - y0,  fy0 = 1.f - fy1;

    OutType v00 = img.at<InType>(y0, x0);
    OutType v01 = img.at<InType>(y0, x0+1);
    OutType v10 = img.at<InType>(y0+1, x0);
    OutType v11 = img.at<InType>(y0+1, x0+1);

    return fx0*fy0*v00 + fx1*fy0*v01 +
           fx0*fy1*v10 + fx1*fy1*v11;
}


} // namespace imgproc

#endif // imgproc_binterpolate_hpp_included_
