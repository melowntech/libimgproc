/**
 * @file rastermask.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask.
 */

#ifndef imgproc_rastermask_hpp_included_
#define imgproc_rastermask_hpp_included_

#include "rastermask/bitfield.hpp"
#include "rastermask/quadtree.hpp"

namespace imgproc {

// the default rastermask is the bit-field based one
typedef bitfield::RasterMask RasterMask;

} // namespace imgproc

#endif // imgproc_rastermask_hpp_included_
