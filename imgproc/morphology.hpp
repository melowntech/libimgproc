#ifndef IMGPROC_MORPHOLOGY_HPP
#define IMGPROC_MORPHOLOGY_HPP

#include "math/geometry_core.hpp"

#if IMGPROC_HAS_OPENCV
#include <opencv2/core/core.hpp>
#endif

namespace imgproc {

#if IMGPROC_HAS_OPENCV
template<typename MatType>
void erode(cv::Mat &mat, int kernelSize = 3)
{
    cv::Mat result(mat.rows, mat.cols, mat.type());

    kernelSize /= 2;

    for (int y = 0; y < mat.rows; y++)
    for (int x = 0; x < mat.cols; x++)
    {
        MatType minimum = mat.at<MatType>(y, x);

        for (int i = -kernelSize; i <= kernelSize; i++) {
            if (y+i < 0 || y+i >= mat.rows) continue;

            for (int j = -kernelSize; j <= kernelSize; j++) {
                if (x+j < 0 || x+j >= mat.cols) continue;

                MatType value = mat.at<MatType>(y+i, x+j);
                minimum = std::min(value, minimum);
            }
        }

        result.at<MatType>(y, x) = minimum;
    }

    result.copyTo(mat);
}


template<typename MatType>
void dilate(cv::Mat &mat, int kernelSize = 3)
{
    cv::Mat result(mat.rows, mat.cols, mat.type());

    kernelSize /= 2;

    for (int y = 0; y < mat.rows; y++)
    for (int x = 0; x < mat.cols; x++)
    {
        MatType maximum = mat.at<MatType>(y, x);

        for (int i = -kernelSize; i <= kernelSize; i++) {
            if (y+i < 0 || y+i >= mat.rows) continue;

            for (int j = -kernelSize; j <= kernelSize; j++) {
                if (x+j < 0 || x+j >= mat.cols) continue;

                MatType value = mat.at<MatType>(y+i, x+j);
                maximum = std::max(value, maximum);
            }
        }

        result.at<MatType>(y, x) = maximum;
    }

    result.copyTo(mat);
}
#endif

} // namespace imgproc

#endif // IMGPROC_MORPHOLOGY_HPP