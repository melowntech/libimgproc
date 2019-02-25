/**
 * Copyright (c) 2018 Melown Technologies SE
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

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/call.hpp>
#include <boost/python/scope.hpp>

#include <stdint.h>

#include "dbglog/dbglog.hpp"

#include "pysupport/package.hpp"
#include "pysupport/class.hpp"

#include "../georeferencing.hpp"
#include "../rasterizer.hpp"

#include "zbuffer.hpp"

namespace bp = boost::python;

namespace imgproc { namespace py {

template <typename T>
bp::class_<imgproc::Georeferencing2_<T>> georeferencing2(const char *name)
{
    using namespace bp;
    typedef imgproc::Georeferencing2_<T> Georeferencing2;

    auto cls = class_<Georeferencing2>(name, init<Georeferencing2>())
        ;
    pysupport::def_readwrite(cls, "ul", &Georeferencing2::ul);
    pysupport::def_readwrite(cls, "ur", &Georeferencing2::ur);
    pysupport::def_readwrite(cls, "lr", &Georeferencing2::lr);
    pysupport::def_readwrite(cls, "ll", &Georeferencing2::ll);

    return cls;
}

template <typename T>
bp::class_<imgproc::Georeferencing3_<T>> georeferencing3(const char *name)
{
    using namespace bp;
    typedef imgproc::Georeferencing3_<T> Georeferencing3;

    auto cls = class_<Georeferencing3>(name, init<Georeferencing3>())
        ;
    pysupport::def_readwrite(cls, "ul", &Georeferencing3::ul);
    pysupport::def_readwrite(cls, "ur", &Georeferencing3::ur);
    pysupport::def_readwrite(cls, "lr", &Georeferencing3::lr);
    pysupport::def_readwrite(cls, "ll", &Georeferencing3::ll);

    return cls;
}

template <typename PointType>
void Rasterizer_rasterize(imgproc::Rasterizer &rasterizer
                          , const PointType &a, const PointType &b
                          , const PointType &c, const bp::object &callable)
{
    rasterizer(a, b, c, [&callable](int x, int y, float z) {
            callable(x, y, z);
        });
}

void Rasterizer_rasterize_raw2(imgproc::Rasterizer &rasterizer
                               , float a1, float a2
                               , float b1, float b2
                               , float c1, float c2
                               , const bp::object &callable)
{
    rasterizer(a1, a2, 0.f, b1, b2, 0.f, c1, c2, 0.f
               , [&callable](int x, int y, float z) {
                   callable(x, y, z);
               });
}

void Rasterizer_rasterize_raw3(imgproc::Rasterizer &rasterizer
                               , float a1, float a2, float a3
                               , float b1, float b2, float b3
                               , float c1, float c2, float c3
                               , const bp::object &callable)
{
    rasterizer(a1, a2, a3, b1, b2, b3, c1, c2, c3
               , [&callable](int x, int y, float z) {
                   callable(x, y, z);
               });
}


} } // namespace imgproc::py

BOOST_PYTHON_MODULE(melown_imgproc)
{
    using namespace bp;
    namespace py = imgproc::py;

    py::georeferencing2<int>("Georeferencing2i");
    py::georeferencing2<double>("Georeferencing2");

    py::georeferencing3<int>("Georeferencing3i");
    py::georeferencing3<double>("Georeferencing3");

    auto Rasterizer = class_<imgproc::Rasterizer>
        ("Rasterizer", init<const math::Extents2i&>())
        .def(init<const math::Size2&>())
        .def(init<int, int>())
        .def("__call__", &py::Rasterizer_rasterize<math::Point2>)
        .def("__call__", &py::Rasterizer_rasterize<math::Point2i>)
        .def("__call__", &py::Rasterizer_rasterize<math::Point3>)
        .def("__call__", &py::Rasterizer_rasterize<math::Point3i>)
        .def("__call__", &py::Rasterizer_rasterize_raw2)
        .def("__call__", &py::Rasterizer_rasterize_raw3)
        ;

    // pull in zbuffer stuff, needs OpenCV and NumPy
    py::registerZBuffer();
}

namespace imgproc { namespace py {
PYSUPPORT_MODULE_IMPORT(imgproc)
} } // namespace imgproc::py
