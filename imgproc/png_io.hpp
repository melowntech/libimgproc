/**
 * @file png.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Fix for gil png support (uses macros that were removed from libpng > 1.2
 */

#ifndef imgproc_png_io_hpp_included_
#define imgproc_png_io_hpp_included_

#include <png.h>

// define missing macros
#ifndef png_infopp_NULL
#define png_infopp_NULL (png_infopp)NULL
#endif // png_infopp_NULL

#ifndef int_p_NULL
#define int_p_NULL (int*)NULL
#endif // int_p_NULL

// safe to include gil

#include <boost/gil/extension/io/png_io.hpp>

#endif // imgproc_png_io_hpp_included_
