/**
 * Copyright (c) 2020 Melown Technologies SE
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
#include <boost/python/enum.hpp>
#include <boost/python/scope.hpp>

#include <opencv2/core/core.hpp>

#include <stdint.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/ndarrayobject.h>

#include "dbglog/dbglog.hpp"

#include "utility/enum-io.hpp"

#include "pysupport/package.hpp"
#include "pysupport/class.hpp"
#include "pysupport/enum.hpp"

#include "../georeferencing.hpp"
#include "../rasterizer.hpp"

namespace bp = boost::python;

namespace imgproc { namespace py {

void* importNumpy()
{
    if (!PyArray_API) {
        import_array();
        if (!PyArray_API) {
            throw bp::error_already_set();
        }
    }
    return nullptr;
}

namespace {

NPY_TYPES cv2numpy(int depth)
{
    // determine output datatype automatically
    switch (depth) {
    case CV_8U:  return NPY_UBYTE;
    case CV_16U: return NPY_USHORT;
    case CV_16S: return NPY_SHORT;
    case CV_32S: return NPY_INT;
    case CV_32F: return NPY_FLOAT;
    case CV_64F: return NPY_DOUBLE;

    default:
        LOGTHROW(err2, std::logic_error)
            << "Unsupported OpenCV depth " << depth << " for numpy.";
    }
    return {}; // never reached
}

struct MatHolder {
    cv::Mat mat;

    MatHolder(const cv::Mat &mat) : mat(mat) {}
    std::string repr() const {
        std::ostringstream os;
        os << "MatHolder{dims=";
        for (int i(0); i < mat.dims; ++i) {
            if (i) { os << "x"; }
            os << mat.size[i];
        }

        os << ", channels=" << mat.channels()
           << ", data=" << static_cast<const void*>(mat.data)
           << "}";
        return os.str();
    }
};

} // namespace

bp::object asNumpyArray(const cv::Mat &mat)
{
    std::vector<npy_intp> dims;
    std::vector<npy_intp> strides;

    {
        for (int i(0); i < mat.dims; ++i) {
            dims.push_back(mat.size[i]);
        }
        if (mat.channels() > 1) { dims.push_back(mat.channels()); }

        for (int i(0); i < mat.dims; ++i) {
            strides.push_back(mat.step[i]);
        }
        if (mat.channels() > 1) { strides.push_back(mat.elemSize1()); }
    }

    auto array(PyArray_NewFromDescr
               (&PyArray_Type, PyArray_DescrFromType(cv2numpy(mat.depth()))
                , dims.size(), dims.data(), strides.data()
                , mat.data, 0, nullptr));
    if (!array) { bp::throw_error_already_set(); }

    if (PyArray_SetBaseObject
        (reinterpret_cast<PyArrayObject*>(array)
         , boost::python::incref(bp::object(MatHolder(mat)).ptr())))
    {
        bp::throw_error_already_set();
    }

    return bp::object(bp::handle<>(array));
}

void registerNumpy()
{
    using namespace bp;

    auto MatHolder_class = class_<MatHolder>
        ("MatHolder", no_init)
        .def("__repr__", &MatHolder::repr)
        ;
}

} } // namespace imgproc::py
