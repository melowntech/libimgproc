/**
 * Copyright (c) 2019 Melown Technologies SE
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

#include "dbglog/dbglog.hpp"

#include "utility/enum-io.hpp"

#include "numpy.hpp"

#include "pysupport/package.hpp"
#include "pysupport/class.hpp"
#include "pysupport/enum.hpp"

#include "../georeferencing.hpp"
#include "../rasterizer.hpp"

#ifdef PYIMGPROC_HAS_GEOMETRY
#  include "geometry/mesh.hpp"
#endif

namespace bp = boost::python;

namespace imgproc { namespace py {

UTILITY_GENERATE_ENUM(ZBufferCompare,
                      ((greater))
                      ((less))
                      )

template <ZBufferCompare comparator, typename T> struct Comparator {};

template <typename T>
struct Comparator<ZBufferCompare::greater, T> {
    bool compare(const T &l, const T &r) { return l > r; }
};

template <typename T>
struct Comparator<ZBufferCompare::less, T> {
    bool compare(const T &l, const T &r) { return l < r; }
};

class ZBufferArrayBase {
public:
    virtual void rasterize(float a1, float a2, float a3
                           , float b1, float b2, float b3
                           , float c1, float c2, float c3) = 0;

    virtual ~ZBufferArrayBase() {}
};

template <typename T, ZBufferCompare comparator>
class ZBufferArray
    : public ZBufferArrayBase
    , private Comparator<comparator, T>
{
public:
    ZBufferArray(T *data, const math::Size2 &dataSize
                 , std::size_t dataStep)
        : extents_(0, 0, dataSize.width, dataSize.height)
        , data_(dataSize.height, dataSize.width, data, dataStep)
        , r_(extents_)
    {}

    virtual void rasterize(float a1, float a2, float a3
                           , float b1, float b2, float b3
                           , float c1, float c2, float c3)
    {
        r_(a1, a2, a3, b1, b2, b3, c1, c2, c3, *this);
    }

    void operator()(int x, int y, float z) {
        auto &value(data_(y, x));
        if (this->compare(z, value)) { value = z; }
    }

private:
    const math::Extents2i extents_;
    cv::Mat_<T> data_;

    Rasterizer r_;
};

class ZBuffer {
public:
    ZBuffer(const bp::object &data
            , ZBufferCompare compare = ZBufferCompare::greater);

    template <typename T>
    void rasterize(const math::Point3_<T> &a, const math::Point3_<T> &b
                   , const math::Point3_<T> &c)
    {
        array_->rasterize(a(0), a(1), a(2), b(0), b(1), b(2)
                          , c(0), c(1), c(2));
    }

    template <typename T>
    void rasterizeRaw(T a1, T a2, T a3, T b1, T b2, T b3, T c1, T c2, T c3)
    {
        array_->rasterize(a1, a2, a3, b1, b2, b3, c1, c2, c3);
    }

#ifdef PYIMGPROC_HAS_GEOMETRY
    template <typename T>
    void rasterizeFaces(const std::vector<math::Point3_<T>> &vertices
                        , const geometry::Face::list &faces)
    {
        for (const auto &face : faces) {
            rasterize(vertices[face.a], vertices[face.b], vertices[face.c]);
        }
    }

    void rasterizeMesh(const geometry::Mesh &mesh)
    {
        rasterizeFaces(mesh.vertices, mesh.faces);
    }
#endif

private:
    bp::object data_;
    std::shared_ptr<ZBufferArrayBase> array_;
};

template <typename T, ZBufferCompare compare>
std::shared_ptr<ZBufferArrayBase>
makeArray(void *data, const math::Size2 &dataSize, std::size_t dataStep)
{
    return std::make_shared<ZBufferArray<T, compare>>
        (static_cast<T*>(data), dataSize, dataStep);
}

template <typename T>
std::shared_ptr<ZBufferArrayBase>
makeArray(void *data, const math::Size2 &dataSize, std::size_t dataStep
          , ZBufferCompare compare, std::size_t elementStride)
{
    if (elementStride != sizeof(T)) {
        LOGTHROW(err1, std::logic_error)
            << "ZBuffer: Individual array elements inside row must "
            "be tightly packed.";
    }

    switch (compare) {
    case ZBufferCompare::greater:
        return makeArray<T, ZBufferCompare::greater>
            (data, dataSize, dataStep);

    case ZBufferCompare::less:
        return makeArray<T, ZBufferCompare::less>
            (data, dataSize, dataStep);
    }

    LOGTHROW(err1, std::logic_error)
        << "ZBuffer:: Invalid compare operator with enumeration value "
        << static_cast<int>(compare) << ".";
    throw;
}

ZBuffer::ZBuffer(const bp::object &data, ZBufferCompare compare)
    : data_(data)
{
    importNumpy();

    if (PyArray_Check(data.ptr())) {
        auto a(reinterpret_cast<PyArrayObject*>(data.ptr()));
        if (PyArray_NDIM(a) != 2) {
            LOGTHROW(err1, std::logic_error)
                << "ZBuffer must be a 2D array; provided array has "
                << PyArray_NDIM(a) << " dimenstion.";
        }

        math::Size2 size(PyArray_DIM(a, 1), PyArray_DIM(a, 0));
        const auto *strides(PyArray_STRIDES(a));

        const auto dtype(PyArray_DTYPE(a));
        if (dtype->byteorder != '=') {
            LOGTHROW(err1, std::logic_error)
                << "ZBuffer supports only native byte order.";
        }

        switch (dtype->type) {
        case 'f':
            array_ = makeArray<float>(PyArray_DATA(a), size, strides[0]
                                      , compare, strides[1]);
            return;

        case 'd':
            array_ = makeArray<double>(PyArray_DATA(a), size, strides[0]
                                       , compare, strides[1]);
            return;
       }

        LOGTHROW(err1, std::logic_error)
            << "ZBuffer must be array of floats or doubles.";
    }

    LOGTHROW(err1, std::logic_error)
        << "ZBuffer does not support objects of type "
        << data.ptr()->ob_type->tp_name << " as the underlying data array.";
}

void registerZBuffer()
{
    using namespace bp;

    auto ZBuffer_class = class_<ZBuffer>
        ("ZBuffer", init<const bp::object&>())
        .def(init<const bp::object&, ZBufferCompare>())
        .def("__call__", &ZBuffer::rasterize<float>)
        .def("__call__", &ZBuffer::rasterize<double>)
        .def("__call__", &ZBuffer::rasterizeRaw<float>)
        .def("__call__", &ZBuffer::rasterizeRaw<double>)

        // geometry support
#ifdef PYIMGPROC_HAS_GEOMETRY
        .def("__call__", &ZBuffer::rasterizeFaces<float>)
        .def("__call__", &ZBuffer::rasterizeFaces<double>)
        .def("__call__", &ZBuffer::rasterizeMesh)
#endif
        ;

    // inject ZBuffer::Compare enum
    {
        bp::scope scope(ZBuffer_class);
        pysupport::fillEnum<ZBufferCompare>
            ("Compare", "Compare operator.");
    }
}

} } // namespace imgproc::py
