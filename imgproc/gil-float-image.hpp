/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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

typedef pixel<double, gray_layout_t> grayd_pixel_t;
typedef image<grayd_pixel_t, false> grayd_image_t;
typedef grayd_image_t::view_t grayd_view_t;
typedef grayd_image_t::const_view_t grayd_const_view_t;

} } // namespace boost::gil

#endif // imgproc_gil_float_image_hpp_included_
