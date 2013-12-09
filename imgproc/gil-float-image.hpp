#ifndef imgproc_gil_float_image_hpp_included_
#define imgproc_gil_float_image_hpp_included_

/** Inject float version of image to boost::gil namespace
 *
 *  This type of image cannot be used in color convertion algorithms since its
 *  values are unbound.
 */
namespace boost { namespace gil {

typedef pixel<float, gray_layout_t> gray_float_pixel_t;
typedef image<gray_float_pixel_t, false> gray_float_image_t;
typedef gray_float_image_t::view_t gray_float_view_t;
typedef gray_float_image_t::const_view_t gray_float_const_view_t;

} } // namespace boost::gil

#endif // imgproc_gil_float_image_hpp_included_
