/**
 * @file clahe.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Contrast-limited adaptive histogram equalization (CLAHE), based on
 * time-proven Karel Zuiderveld 1994 implementation and adapted to support
 * multi-channel arbitrary-dimension OpenCV images.
 */

#ifndef IMGPROC_CLAHE_HPP
#define IMGPROC_CLAHE_HPP

#include <opencv2/core/core.hpp>

namespace imgproc {

/**
 * @brief perform CLAHE on an image
 * @details This function performs contrast-limited adaptive histogram
 * equalization on an input image. clipLimit values lesser then 0 disable
 * contrast limiting and result in the standard AHE algorithm. The input
 * image can be of type CV_8UC1, CV_8UC3, CV_16UC1 or CV_16UC3. If a 3 channel
 * image is supplied, CLAHE is applied to the intensity channel only and
 * chromatic information is left untouched. */
void CLAHE( const cv::Mat & src, cv::Mat & dst, const int regionSize,
            float clipLimit = -1.0 );

} // namespace imgproc

#endif // IMGPROC_CLAHE_HPP
