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

#include <sstream>
#include <string>
#include <vector>
#include <mutex>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/call.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/scope.hpp>

#include <stdint.h>

#include "dbglog/dbglog.hpp"

#include "pysupport/package.hpp"
#include "pysupport/class.hpp"

#include "../georeferencing.hpp"

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

} } // namespace imgproc::py

BOOST_PYTHON_MODULE(melown_imgproc)
{
    using namespace bp;
    namespace py = imgproc::py;

    py::georeferencing2<int>("Georeferencing2i");
    py::georeferencing2<double>("Georeferencing2");

    py::georeferencing3<int>("Georeferencing3i");
    py::georeferencing3<double>("Georeferencing3");
}

namespace imgproc { namespace py {

namespace {
std::once_flag onceFlag;
} // namespace

boost::python::object import()
{
    std::call_once(onceFlag, [&]()
    {
        typedef bp::handle< ::PyObject> Handle;
        Handle module(PyInit_melown_imgproc());

        auto package(pysupport::package());

        if (::PyModule_AddObject(package.ptr(), "imgproc"
                                 , bp::incref(module.get())) == -1)
        {
            LOG(err2) << "PyModule_AddObject failed";
        }

        auto sys(bp::import("sys"));
        sys.attr("modules")["melown.imgproc"] = module;
    });

    return bp::import("melown.imgproc");
}

} } // namespace imgproc::py
