#ifndef imgproc_gil_float_image_hpp_included_
#define imgproc_gil_float_image_hpp_included_

#include <boost/gil/pixel.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>

/** Inject float version of image to boost::gil namespace
 *
 *  This type of image cannot be used in color convertion algorithms since its
 *  values are unbound.
 */
namespace boost { namespace gil {

typedef pixel<float, gray_layout_t> grayf_pixel_t;
typedef image<grayf_pixel_t, false> grayf_image_t;
typedef grayf_image_t::view_t grayf_view_t;
typedef grayf_image_t::const_view_t grayf_const_view_t;

typedef pixel<float, gray_layout_t> grayd_pixel_t;
typedef image<grayd_pixel_t, false> grayd_image_t;
typedef grayd_image_t::view_t grayd_view_t;
typedef grayd_image_t::const_view_t grayd_const_view_t;

} } // namespace boost::gil

#endif // imgproc_gil_float_image_hpp_included_
