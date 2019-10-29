/**
 * Copyright (c) 2019 Melown Technologies SE
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

#include "imgwarp.hpp"
#include "utility/openmp.hpp"
#include "math/math.hpp"

// Computes a value of given image functor at floating-point coordinates,
// using bilinear interpolation.
template <typename TImg>
static float interpolate(const float x, const float y, const TImg& img) {
    const int x0 = int(std::floor(x));
    const int y0 = int(std::floor(y));
    const float fx = x - x0;
    const float fy = y - y0;

    const float v00 = img(x0, y0);
    const float v01 = img(x0 + 1, y0);
    const float v10 = img(x0, y0 + 1);
    const float v11 = img(x0 + 1, y0 + 1);

    const float w0 = v00 + (v01 - v00) * fx;
    const float w1 = v10 + (v11 - v10) * fx;

    return w0 + (w1 - w0) * fy;
}

// Returns i % n for i>0 and i % n + n for i<0
static int positiveMod(const int i, const int n) {
     return (i % n + n) % n;
}

static float handleBorderAndIntepolate(const cv::Mat& img, const float u, const float v,
                                       const int border, const uchar borderValue) {
    const int maxX = img.cols - 1;
    const int maxY = img.rows - 1;
    switch (border) {
    case cv::BORDER_CONSTANT:
        return interpolate(u, v, [maxX, maxY, borderValue, &img](const int x, const int y) {
            if (uint(x) <= uint(maxX) && uint(y) <= uint(maxY)) {
                return img.at<uchar>(y, x);
            } else {
                return borderValue;
            }
        });
    case cv::BORDER_REPLICATE:
        return interpolate(u, v, [maxX, maxY, &img](const int x, const int y) {
            return img.at<uchar>(math::clamp(y, 0, maxY), math::clamp(x, 0, maxX));
        });
    case cv::BORDER_REFLECT:
        return interpolate(u, v, [maxX, maxY, &img](const int x, const int y) {
            const int x1 = x - (x < 0) * (2 * x + 1) + (x > maxX) * (2 * maxX - 2 * x + 1);
            const int y1 = y - (y < 0) * (2 * y + 1) + (y > maxY) * (2 * maxY - 2 * y + 1);
            return img.at<uchar>(math::clamp(y1, 0, maxY), math::clamp(x1, 0, maxX));
        });
    case cv::BORDER_WRAP:
        return interpolate(u, v, [&img](const int x, const int y) {
            return img.at<uchar>(positiveMod(y, img.rows), positiveMod(x, img.cols));
        });
    case cv::BORDER_REFLECT_101:
        return interpolate(u, v, [maxX, maxY, &img](const int x, const int y) {
            const int x1 = x - (x < 0) * 2 * x + (x > maxX) * (2 * maxX - 2 * x);
            const int y1 = y - (y < 0) * 2 * y + (y > maxY) * (2 * maxY - 2 * y);
            return img.at<uchar>(math::clamp(y1, 0, maxY), math::clamp(x1, 0, maxX));
        });
    default:
        throw std::runtime_error("Unknown border mode " + std::to_string(border));
    }
}

void imgproc::warpPerspective(const cv::Mat& src, cv::Mat& dst, const cv::Mat& H, const cv::Size dsize,
    const int border, const uchar borderValue) {
    dst.create(dsize, CV_8U);

    cv::Mat_<double> Hinv;
    cv::invert(H, Hinv);

    UTILITY_OMP(parallel for)
    for (int y = 0; y < dst.rows; y++) {
        for (int x = 0; x < dst.cols; x++) {
            const double iw = 1.0 / (Hinv(2, 0) * x + Hinv(2, 1) * y + Hinv(2, 2));

            const float u = (Hinv(0, 0) * x + Hinv(0, 1) * y + Hinv(0, 2)) * iw;
            const float v = (Hinv(1, 0) * x + Hinv(1, 1) * y + Hinv(1, 2)) * iw;

            dst.at<uchar>(y, x) = handleBorderAndIntepolate(src, u, v, border, borderValue);
        }
    }
}
