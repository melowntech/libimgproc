#include "dbglog/dbglog.hpp"

#include "./transform.hpp"
#include "./cvmat.hpp"

namespace imgproc { namespace quadtree {

namespace {

inline math::Point2 trans(const Matrix2x3 &tr, int x, int y)
{
    return {
        tr(0, 0) * x + tr(0, 1) * y + tr(0, 2)
        , tr(1, 0) * x + tr(1, 1) * y + tr(1, 2)
     };
}

} // namespace

RasterMask transform(const RasterMask &mask, const math::Size2 &size
                     , const Matrix2x3 &trafo)
{
    // kernel sizes (from scaling factor)
    double kw(trafo(0, 0) / 2.0);
    double kh(trafo(1, 1) / 2.0);

    auto m(asCvMat(mask));

    auto clamp([](int i, int max) -> int
    {
        if (i < 0) { return 0; }
        if (i > max) { return max; }
        return i;
    });

    auto scan([&](const math::Point2 &p) -> bool
    {
        for (int y(clamp(std::floor(p(1) - kh), m.rows))
                 , ey(clamp(std::ceil(p(1) + kh), m.rows));
             y != ey; ++y)
        {
            for (int x(clamp(std::floor(p(0) - kw), m.cols))
                     , ex(clamp(std::ceil(p(0) + kw), m.cols));
                 x != ex; ++x)
            {
                if (!m.at<std::uint8_t>(y, x)) {
                    return false;
                }
            }
        }
        return true;
    });

    RasterMask out(size, RasterMask::EMPTY);
    for (int j(0); j < size.height; ++j) {
        for (int i(0); i < size.width; ++i) {
            if (scan(trans(trafo, i, j))) {
                out.set(i, j);
            }
        }
    }

    // done
    return out;
}

} } // namespace imgproc::quadtree
