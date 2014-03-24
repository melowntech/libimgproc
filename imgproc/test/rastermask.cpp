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
