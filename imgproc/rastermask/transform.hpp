#ifndef imgproc_rastermask_transform_hpp_included_
#define imgproc_rastermask_transform_hpp_included_

#include <opencv2/core/core.hpp>

#include "./quadtree.hpp"

namespace imgproc { namespace quadtree {

typedef boost::numeric::ublas::matrix
<double, boost::numeric::ublas::row_major
 , boost::numeric::ublas::bounded_array<double, 6> > Matrix2x3;

/** Transforms raster mask to new mask of size size by given transformation
 *  matrix.
 */
RasterMask transform(const RasterMask &mask, const math::Size2 &size
                     , const Matrix2x3 &trafo);

} } // namespace imgproc::quadtree

#endif // imgproc_rastermask_transform_hpp_included_
