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

#ifndef imgproc_detail_contours_hpp_included_
#define imgproc_detail_contours_hpp_included_

#include <cstdint>
#include <map>

#include <boost/noncopyable.hpp>

#include "math/geometry_core.hpp"

namespace imgproc { namespace detail {

struct FindContoursImpl : boost::noncopyable {
    typedef std::uint8_t CellType;
    typedef std::map<math::Point2i, CellType> AmbiguousCells;

    AmbiguousCells ambiguousCells_;

    CellType ambiguousType(const math::Point2i &p, CellType type) {
        auto fambiguousCells(ambiguousCells_.find(p));
        if (fambiguousCells == ambiguousCells_.end()) {
            ambiguousCells_.insert(AmbiguousCells::value_type(p, type));
            return type;
        }
        return fambiguousCells->second;
    }
};

} } // namespace imgproc::detail

#endif // imgproc_detail_contours_hpp_included_
