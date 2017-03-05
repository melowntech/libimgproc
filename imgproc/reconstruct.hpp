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

} // namespace imgproc

#endif // imgproc_filtering_generic_hpp_included_
