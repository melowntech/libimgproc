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
 * @file reconstruct.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Pixel valud reconstruction using generic const raster accessor.
 */

#ifndef imgproc_reconstruct_hpp_included_
#define imgproc_reconstruct_hpp_included_

#include "./const-raster.hpp"

namespace imgproc {

template<typename ConstRaster, typename Filter2>
typename detail::ReconstructResult<ConstRaster>::type
reconstruct(const ConstRaster &r, const Filter2 &filter
            , const math::Point2 &pos);

// implementation

// Reconstruction core itself
template<typename ConstRaster, typename Filter2>
typename detail::ReconstructResult<ConstRaster>::type
reconstruct(const ConstRaster &raster, const Filter2 &filter
            , const math::Point2 &pos)
{
    // calculate filtering window
    const int x1(std::floor(pos(0) - filter.halfwinx()));
    const int x2(std::ceil (pos(0) + filter.halfwinx()));
    const int y1(std::floor(pos(1) - filter.halfwiny()));
    const int y2(std::ceil (pos(1) + filter.halfwiny()));
    const int total((x2 - x1 + 1) * (y2 - y1 + 1));

    const int numChannels(raster.channels());

    // accumulate values for whole filtering window
    double weightSum[2] = { 0.0, 0.0 };
    double valueSum[2][10] = {
        {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        }
        , {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        }
    };
    int count(0);

    for (int i = y1; i <= y2; ++i) {
        for (int j = x1; j <= x2; ++j) {
            const double weight(filter(j - pos(0), i - pos(1)));
            bool negative(weight < 0.0);

            if (raster.valid(j, i)) {
                const auto &value(raster(j, i));
                for (int k = 0; k < numChannels; ++k) {
                    valueSum[negative][k] += weight * value[k];
                }
                weightSum[negative] += weight;
                ++count;
            }
        }
    }

    // nothing sampled -> undefined pixel
    if (!count) {
        return raster.undefined();
    }

    // compute weight sum based on number of sampled pixels
    double weight((count < total)
                  ? weightSum[0]
                  : weightSum[0] + weightSum[1]);
    if (weight < 1e-15) {
        // weight not sane -> undefined
        return raster.undefined();
    }

    // defined color
    typename ConstRaster::value_type retval;
    if (count < total) {
        // some invalid -> use only pixels with non-negative weights
        for (int i = 0; i < numChannels; ++i) {
            retval[i] = raster.saturate(valueSum[0][i] / weight);
        }
    } else {
        // all valid -> use all pixels
        for (int i = 0; i < numChannels; ++i) {
            retval[i] = raster.saturate
                ((valueSum[0][i] + valueSum[1][i]) / weight);
        }
    }
    return retval;
}

// Reconstruction core itself
template<int MaxChannels = 32, typename ConstRaster
         , typename Filter2, typename WriteChannel>
bool reconstruct(const ConstRaster &raster, const Filter2 &filter
                 , const math::Point2 &pos, const WriteChannel &write)
{
    // calculate filtering window
    const int x1(std::floor(pos(0) - filter.halfwinx()));
    const int x2(std::ceil (pos(0) + filter.halfwinx()));
    const int y1(std::floor(pos(1) - filter.halfwiny()));
    const int y2(std::ceil (pos(1) + filter.halfwiny()));
    const int total((x2 - x1 + 1) * (y2 - y1 + 1));

    const int numChannels(raster.channels());

    // accumulate values for whole filtering window
    double weightSum[2] = { 0.0, 0.0 };
    double valueSum[2][MaxChannels] = { {}, {} };
    int count(0);

    for (int i = y1; i <= y2; ++i) {
        for (int j = x1; j <= x2; ++j) {
            const double weight(filter(j - pos(0), i - pos(1)));
            bool negative(weight < 0.0);

            if (raster.valid(j, i)) {
                const auto &value(raster(j, i));
                for (int k = 0; k < numChannels; ++k) {
                    valueSum[negative][k] += weight * value[k];
                }
                weightSum[negative] += weight;
                ++count;
            }
        }
    }

    // nothing sampled -> undefined pixel
    if (!count) { return false; }

    // compute weight sum based on number of sampled pixels
    double weight((count < total)
                  ? weightSum[0]
                  : weightSum[0] + weightSum[1]);
    if (weight < 1e-15) {
        // weight not sane -> undefined
        return false;
    }

    // defined color
    if (count < total) {
        // some invalid -> use only pixels with non-negative weights
        for (int i = 0; i < numChannels; ++i) {
            write(i, raster.saturate(valueSum[0][i] / weight));
        }
    } else {
        // all valid -> use all pixels
        for (int i = 0; i < numChannels; ++i) {
            write(i, raster.saturate
                  ((valueSum[0][i] + valueSum[1][i]) / weight));
        }
    }
    return true;
}

} // namespace imgproc

#endif // imgproc_filtering_generic_hpp_included_
