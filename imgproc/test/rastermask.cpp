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
#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "imgproc/rastermask/quadtree.hpp"

#include "dbglog/dbglog.hpp"

BOOST_AUTO_TEST_CASE(rastermask_quadtree_invert)
{
    BOOST_TEST_MESSAGE("* Testing QuadTree-based rastermask invertions.");

    using imgproc::quadtree::RasterMask;

    math::Size2 size(1024, 1024);

    // prepare mask with random data
    RasterMask src(size.width, size.height, RasterMask::InitMode::EMPTY);
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(0, 1);
    for (int j(0); j < size.width; ++j) {
        for (int i(0); i < size.height; ++i) {
            src.set(i, j, dist(gen));
        }
    }

    // derive new mask and invert it
    RasterMask dst(src);
    dst.invert();

    // check difference
    for (int j(0); j < size.width; ++j) {
        for (int i(0); i < size.height; ++i) {
            BOOST_REQUIRE(dst.get(i, j) != src.get(i, j));
        }
    }
}
