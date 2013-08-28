/**
 * @file scanconversion.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 *
 * Triangle scan conversion
 */

#ifndef imgproc_scanconversion_hpp_included_
#define imgproc_scanconversion_hpp_included_

#include <vector>
#include <opencv2/core/core.hpp>

namespace imgproc {

//! Triangle scan conversion helper structure
struct Scanline
{
    int y;
    float x1, x2;
    float z1, z2;

    Scanline(int y, float x1, float x2, float z1, float z2);
};

//! Converts a triangle to a list of scanlines. The triangle vertices include
//! depth (Z), which is also interpolated.
void scanConvertTriangle(const cv::Point3f pt[3], int ymin, int ymax,
                         std::vector<Scanline>& scanlines);


//! Calls the specified operation for each pixel of a scanline
template<typename Operation>
void processScanline(const Scanline& sl, int xmin, int xmax, Operation op)
{
    float x = ceil(sl.x1);
    if (x < xmin) x = xmin;

    float dz = (sl.z2 - sl.z1) / (sl.x2 - sl.x1);
    float z = sl.z1 + dz*(x - sl.x1);

    while (x < sl.x2 && x < xmax) {
        op((int) x, sl.y, z);
        x += 1.0f;
        z += dz;
    }
}


} // namespace imgproc

#endif // imgproc_scanconversion_hpp_included_
