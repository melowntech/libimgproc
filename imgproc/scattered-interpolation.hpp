/**
 * @file scattered-interpolation.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Interpolation on 2D scattered data.
 */

#ifndef imgproc_scattered_interpolation_hpp_included_
#define imgproc_scattered_interpolation_hpp_included_

#include <opencv2/core/core.hpp>

#include "./rastermask.hpp"

namespace imgproc {


/**
 *
 */
void laplaceInterpolate(cv::Mat &data, const imgproc::RasterMask &mask);



} // imgproc

#endif // imgproc_scattered_interpolation_hpp_included_
