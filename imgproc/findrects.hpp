#ifndef imgproc_findrects_hpp_included_
#define imgproc_findrects_hpp_included_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "math/geometry_core.hpp"

namespace imgproc {

/** Finds all rectangles defined as rectangular area of the same color.
 */
template <typename PixelType>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img);

/** Finds all rectangles defined as rectangular area of the same color.
 *  Ignores areas for which filter returns false.
 */
template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img, Filter filter);

/** Finds all squares defined as rectangular area of the same color having the
 *  same with and height.
 */
template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img, Filter filter);

/** Finds all squares defined as rectangular area of the same color having the
 *  same with and height.
 *  Ignores areas for which filter returns false.
 */
template <typename PixelType>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img);

} // namespace imgproc

#include "detail/findrects.impl.hpp"

#endif // imgproc_findrects_hpp_included_
