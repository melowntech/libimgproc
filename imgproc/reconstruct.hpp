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
typename ConstRaster::value_type
reconstruct(const ConstRaster &r, const Filter2 &filter
            , const math::Point2 &pos);


// implementation

// Reconstruction core itself
template<typename ConstRaster, typename Filter2>
typename ConstRaster::value_type
reconstruct(const ConstRaster &raster, const Filter2 &filter
            , const math::Point2 &pos)
{
    // calculate filtering window
    const int x1(std::floor(pos(0) - filter.halfwinx()));
    const int x2(std::ceil (pos(0) + filter.halfwinx()));
    const int y1(std::floor(pos(1) - filter.halfwiny()));
    const int y2(std::ceil (pos(1) + filter.halfwiny()));

    const int numChannels(raster.channels());

    // accumulate values for whole filtering window
    double weightSum(0), valueSum[10] = { 0.0 };
    for (int i = y1; i <= y2; ++i) {
        for (int j = x1; j <= x2; ++j) {
            const double weight(filter(j - pos(0), i - pos(1)));

            if (raster.valid(j, i)) {
                const auto &value(raster(j, i));
                for (int k = 0; k < numChannels; ++k) {
                    valueSum[k] += weight * value[k];
                }
                weightSum += weight;
            }
        }
    }

    // calculate and return result
    typename ConstRaster::value_type retval;
    for (int i = 0; i < numChannels; ++i) {
        if (weightSum > 1e-15) {
            retval[i] = raster.saturate(valueSum[i] / weightSum);
        } else {
            retval[i] = raster.undefined();
        }
    }
    return retval;
}

} // namespace imgproc

#endif // imgproc_filtering_generic_hpp_included_
