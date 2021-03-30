/**
 * Copyright (c) 2021 Melown Technologies SE
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
 * @file sharpen.cpp
 * @author Matyas Hollmann <matyas.hollmann@melowntech.com>
 *
 * Image sharpening based on doubleUSM approach
 * (https://cc-extensions.com/products/doubleusm/)
 */

#include <opencv2/imgproc/imgproc.hpp>

#include "dbglog/dbglog.hpp"

#include "sharpen.hpp"

namespace imgproc {

// double UnSharp Mask (doubleUSM)
cv::Mat_<std::uint8_t> doubleUSM(const cv::Mat_<std::uint8_t>& intensity,
                                 const cv::Mat_<std::uint8_t>& lowContrastMask,
                                 const SharpenParams& params)
{
    cv::Mat_<std::uint8_t> blurred;
    cv::GaussianBlur(intensity, blurred, cv::Size(params.kSize_, params.kSize_),
                     1.0);

    cv::Mat_<std::uint8_t> res(intensity.size());

    int nRows = intensity.rows;
    int nCols = intensity.cols;

    // blurred and res matrices are continuous
    if (intensity.isContinuous() && lowContrastMask.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    const std::uint8_t* orig, *blur, *mask;
    std::uint8_t* r;
    // sharpen by adding weighted high frequencies (difference between original and
    // its smoothened version)
    for (int y = 0; y < nRows; ++y)
    {
        orig = intensity.ptr<std::uint8_t>(y);
        blur = blurred.ptr<std::uint8_t>(y);
        mask = lowContrastMask.ptr<std::uint8_t>(y);
        r = res.ptr<std::uint8_t>(y);

        for (int x = 0; x < nCols; ++x)
        {
            // apply threshold
            if (!mask[x])
            {
                int diff = orig[x] - blur[x];
                // lighten or darken
                float weight = diff < 0 ? params.darkAmount_ : params.lightAmount_;

                r[x] = cv::saturate_cast<std::uint8_t>(orig[x] + std::round(weight * diff));
            }
            else { // just copy original
                r[x] = orig[x];
            }
        }
    }

    return res;
}


cv::Mat sharpen(const cv::Mat& img, const SharpenParams& params, bool isYCrCb)
{
    if (params.kSize_ <= 0 || !(params.kSize_ & 1))
    {
        LOGTHROW(err3, std::runtime_error)
            << "Invalid kernel size: " << params.kSize_
            << ", kernel size has to be a positive odd integer.";
    }

    if (isYCrCb && img.channels() != 3)
    {
        LOGTHROW(err3, std::runtime_error)
            << img.channels()
            << "-channel image can't be in the YCrCb color space.";
    }

    if (img.depth() != CV_8U)
    {
        LOGTHROW(err3, std::runtime_error)
            << "Unsupported image depth: " << img.depth();
    }

    std::vector<cv::Mat_<std::uint8_t>> ycrcb;
    cv::Mat alpha;
    {
        cv::Mat tmp = img;
        switch (img.channels())
        {
            case 4:
                cv::split(img, ycrcb);

                alpha = ycrcb.back();
                ycrcb.pop_back();

                cv::merge(ycrcb, tmp);
                ycrcb.clear();
                // fallthrough
            case 3:
                if (!isYCrCb) {
                    cv::cvtColor(tmp, tmp, cv::COLOR_BGR2YCrCb);
                }
                // fallthrough
            case 1:
                cv::split(tmp, ycrcb);
                break;

            default:
                LOGTHROW(err3, std::runtime_error)
                    << "Unexpected number of channels: " << img.channels();
        }
    }

    cv::Mat_<std::uint8_t>& intensity(ycrcb[0]);

    // apply sharpening only to the intensity/gray channel to avoid color noise
    cv::Mat_<std::uint8_t> blurred;
    cv::GaussianBlur(intensity, blurred, cv::Size(params.kSize_, params.kSize_),
                     1.0);

    cv::Mat_<std::uint8_t>
        lowContrastMask(cv::abs(intensity - blurred) < params.threshold_);

    // sharpen
    intensity = doubleUSM(intensity, lowContrastMask, params);

    // merge channels
    cv::Mat res;
    cv::merge(ycrcb, res);

    if (!isYCrCb)
    {
        cv::cvtColor(res, res, cv::COLOR_YCrCb2BGR);
        if (!alpha.empty())
        {
            ycrcb.clear();

            cv::split(res, ycrcb);
            ycrcb.push_back(alpha);

            cv::merge(ycrcb, res);
        }
    }

    return res;
}

} // namespace imgproc
