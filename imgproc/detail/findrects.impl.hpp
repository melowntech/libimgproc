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
#ifndef imgproc_detail_findrects_hpp_included_
#define imgproc_detail_findrects_hpp_included_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "math/geometry_core.hpp"
#include "imgproc/fillrect.hpp"

namespace imgproc {

namespace detail { namespace findrects {

template <typename SizeType> struct SizeTraits;

template <> struct SizeTraits<int> {
    static bool empty(int size) { return !size; }
    static int width(int size) { return size; }
    static int height(int size) { return size; }
};

template <> struct SizeTraits<cv::Vec2i> {
    static bool empty(const cv::Vec2i &size) { return !(size[0] && size[1]); }
    static int width(const cv::Vec2i &size) { return size[0]; }
    static int height(const cv::Vec2i &size) { return size[1]; }
};

template <typename PixelType, typename SizeType, typename Filter>
std::vector<math::Extents2i>
collectRectangles(const cv::Mat &img, cv::Mat &acc, Filter filter)
{
    std::vector<math::Extents2i> rects;

    const auto zero(cv::Scalar(0, 0));

    // collect
    for (int j(img.rows - 1); j >= 0; --j) {
        for (int i(img.cols - 1); i >= 0; --i) {
            auto size(acc.at<SizeType>(j, i));
            if (SizeTraits<SizeType>::empty(size)) { continue; }

            cv::Point2i start(i - SizeTraits<SizeType>::width(size) + 1
                              , j - SizeTraits<SizeType>::height(size) + 1);
            cv::Point2i end(i, j);
            imgproc::fillRectangle(acc, start, end, zero);

            if (filter(img.at<PixelType>(j, i))) {
                rects.emplace_back(start.x, start.y, end.x, end.y);
            }
        }
    }

    return rects;
}

} } // namespace detail::findrects

template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img, Filter filter)
{
    // prepare accumulator image
    cv::Mat acc(img.rows, img.cols, CV_32SC1);

    for (int j(0); j < img.rows; ++j) {
        for (int i(0); i < img.cols; ++i) {
            if (!i || !j) {
                // force top row and left column to be 1
                acc.at<int>(j, i) = 1;
                continue;
            }

            int left(0);
            int up(0);
            int diag(0);

            const auto currentPx(img.at<PixelType>(j, i));
            if (img.at<PixelType>(j, i - 1) == currentPx) {
                left = acc.at<int>(j, i - 1);
            }

            if (img.at<PixelType>(j - 1, i) == currentPx) {
                up = acc.at<int>(j - 1, i);
            }

            if (img.at<PixelType>(j - 1, i - 1) == currentPx) {
                diag = acc.at<int>(j - 1, i - 1);
            }

            if (left && up && diag) {
                // all three neighbours are of the same color
                auto add(std::min({ left, up, diag }));
                acc.at<int>(j, i) = 1 + add;
            }
        }
    }

    return detail::findrects::collectRectangles<PixelType, int>
        (img, acc, filter);
}

template <typename PixelType>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img)
{
    auto lambda([&](const PixelType&) { return true; });
    return findSquares<PixelType, decltype(lambda)>(img, lambda);
}

template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img, Filter filter)
{
    const cv::Vec2i empty(0, 0);

    cv::Mat acc(img.rows, img.cols, CV_32SC2);

    for (int j(0); j < img.rows; ++j) {
        for (int i(0); i < img.cols; ++i) {
            int width(1);
            int height(1);

            auto currentPx(img.at<PixelType>(j, i));
            if (i && (img.at<PixelType>(j, i - 1) == currentPx)) {
                auto left(acc.at<cv::Vec2i>(j, i - 1));

                // something to the left, continue line
                width += left[0];
            }

            if (j && (img.at<PixelType>(j - 1, i) == currentPx)) {
                auto up (acc.at<cv::Vec2i>(j - 1, i));
                // something to the up, try to continue rectangle
                if (up[0] >= width) {
                    height += up[1];
                }
            }

            // write rect size
            acc.at<cv::Vec2i>(j, i) = { width, height };
        }
    }

    return detail::findrects::collectRectangles<PixelType, cv::Vec2i>
        (img, acc, filter);
}

template <typename PixelType>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img)
{
    auto lambda([&](const PixelType&) { return true; });
    return findRectangles<PixelType, decltype(lambda)>(img, lambda);
}

} // namespace imgproc

#endif // imgproc_detail_findrects_hpp_included_
