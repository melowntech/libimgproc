/**
 * @file spectral_analysis.hpp
 * @author Ladislav Horky <ladislav.horky@citationtech.net>
 *
 * Spectral analysis functions
 */

#ifndef imgproc_spectral_analysis_hpp_included_
#define imgproc_spectral_analysis_hpp_included_

#include <opencv2/core/core.hpp>

#include "math/geometry_core.hpp"


namespace imgproc {

void effectiveScale(const cv::Mat & img, float &hscale, float &vscale
                    , float threshold = 0.1);

math::Size2f effectiveScale(const cv::Mat & img, float threshold = 0.1);

// implementation

inline math::Size2f effectiveScale(const cv::Mat & img, float threshold)
{
    float hscale, vscale;
    effectiveScale(img, hscale, vscale, threshold);
    return math::Size2f(hscale, vscale);
}

} //namespace imgproc

#endif //imgproc_spectral_analysis_hpp_included_
