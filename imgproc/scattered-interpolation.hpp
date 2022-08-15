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
/**
 * @file scattered-interpolation.hpp
 * @author Jakub Cerveny <jakub.cerveny@ext.citationtech.net>
 * @author Matyas Hollmann <matyas.hollmann@melowntech.com>
 *
 * Interpolation on 2D scattered data.
 */

#ifndef imgproc_scattered_interpolation_hpp_included_
#define imgproc_scattered_interpolation_hpp_included_

#include <vector>
#include <Eigen/Sparse>
#include <opencv2/core/core.hpp>

#include "dbglog/dbglog.hpp"
#include "utility/gccversion.hpp"
#include "rastermask.hpp"

namespace imgproc {

#if !defined(IMGPROC_HAS_OPENCV) || !defined(IMGPROC_HAS_EIGEN3)
    UTILITY_FUNCTION_ERROR("Laplace interpolation is available only when "
                           "compiled with both OpenCV and Eigen3 libraries.")
#endif

/** Solves the boundary value problem -\Delta u = 0 on elements in the matrix
 *  'data' that correspond to unset elements in 'mask'. Elements corresponding
 *  to set positions in 'mask' are regarded as given data.
 *
 *  The method is described in section 3.8 "Laplace Interpolation" of
 *  Numerical Recipes in C, Third Edition.
 *
 *  typename T_OPT:  floating-point numeric type in which Eigen solves the problem,
 *                   float(faster), double(more precise)
 *  typename T_DATA: numeric type of (per-channel) elements of 'data' matrix,
 *                   if it's an integral type results are rounded before storing
 *  int nChan:       number of channels of 'data' matrix
 *
 *  Example usage: for 'data' matrix of type CV_32FC2 use <float, 2, T_OPT>
 *                 for 'data' matrix of type  CV_8UC3 use <unsigned char, 3, T_OPT>
 */
template<typename T_DATA, int nChan, typename T_OPT = float>
void laplaceInterpolate(cv::Mat &data, const imgproc::RasterMask &mask, double tol = 1e-12)
{
    static_assert(std::is_floating_point<T_OPT>::value,
                  "Floating-point numeric type expected.");

    assert(sizeof(T_DATA) == data.elemSize1() && data.channels() == nChan);

    // round results if 'data' matrix elements are of integral type
    constexpr bool doRound = std::is_integral<T_DATA>::value;

    using SparseMatrix = Eigen::SparseMatrix<T_OPT>;
    using Triplet = Eigen::Triplet<T_OPT>;
    // used to access individual entries in the 'data' matrix
    using cvVec = cv::Vec<T_DATA, nChan>;
    // used to access individual entris in the 'rhs' vector,
    // should be a vector of floating-point types
    using rhsVecT = cv::Vec<T_OPT, nChan>;

    int w = data.cols, h = data.rows;
    // # of variables
    int n = 0;

    auto linearizeID([w](int x, int y) -> int
    {
        return y * w + x;
    });

    auto isInside([w, h](const cv::Point2i& pt) -> bool
    {
        return std::min(pt.x, pt.y) >= 0 && pt.x < w && pt.y < h;
    });

    // optimize only free points, given points should not be a part of the
    // optimization
    std::vector<int> pixelIDs(w * h, -1);
    for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
        if (!mask.get(x, y)) // free point
        {
            // assing ID and increase value of the counter
            pixelIDs[linearizeID(x, y)] = n++;
        }
    }

    if (!n) // nothing to do here
    {
        LOG(debug) << "All points are given, nothing to do.";
        return;
    }

    // assemble the linear system
    LOG(debug) << "Assembling " << n << "x" << n
               << " sparse system. # of channels: " << nChan;

    std::vector<Triplet> coefs;
    // default constructor = initialize with zeros
    std::vector<rhsVecT> rhsVec(n, rhsVecT());
    coefs.reserve(5 * n);

    const static std::array<cv::Point2i, 4> dirs = {{{1, 0}, {-1, 0},
                                                     {0, 1}, {0, -1}}};

    for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
        // index of the unknown
        int k = pixelIDs[linearizeID(x, y)];

        // given point
        if (k < 0) { continue; }

        cv::Point2i cur{x, y};
        // # of neighbors
        int nNeighs = 0;
        for (const auto& dir : dirs)
        {
            cv::Point2i tmp = cur + dir;
            if (!isInside(tmp)) { continue; }
            ++nNeighs;

            // index of the neighboring unknown
            int t = pixelIDs[linearizeID(tmp.x, tmp.y)];
            if (t < 0) // neighbor is a given point
            {
                // convert 'data' to rhsVecT
                rhsVec[k] += rhsVecT(data.at<cvVec>(tmp));
            }
            else // neighbor is a free point
            {
                coefs.emplace_back(k, t, static_cast<T_OPT>(-1.0));
            }
        }
        coefs.emplace_back(k, k, static_cast<T_OPT>(nNeighs));
    }

    SparseMatrix mat(n, n);
    mat.setFromTriplets(coefs.begin(), coefs.end());

    LOG(debug) << "Matrix constructed.";

    using Precond = Eigen::DiagonalPreconditioner<T_OPT>;
    using Solver = Eigen::BiCGSTAB<SparseMatrix, Precond>;
    using EigVecX = Eigen::Matrix<T_OPT, Eigen::Dynamic, 1>;

    // solve the system
    Solver solver(mat);
    solver.setTolerance(tol);

    EigVecX rhs(n), sln(n);
    for (int i = 0; i < nChan; ++i)
    {
        for (int j = 0; j < n; ++j) {
            rhs(j) = rhsVec[j](i);
        }

        LOG(debug) << "Solving system with rhs = channel " << (i + 1)
                   << " out of " << nChan;

        sln = solver.solve(rhs);

        LOG(debug) << "#iterations: " << solver.iterations();
        LOG(debug) << "estimated error: " << solver.error();
        LOG(debug) << "min: " << sln.minCoeff() << ", max: " << sln.maxCoeff();

        // for integral types we should round the results
        // TODO: in C++17 use if constexpr
        if (doRound)
        {
            for (int j = 0; j < n; ++j) {
               sln(j) = std::round(sln(j));
            }
        }

        // store solution of the i-th channel
        for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            int id = pixelIDs[linearizeID(x, y)];
            if (id >= 0) // free point
            {
                data.at<cvVec>(y, x)(i) = sln(id);
            }
        }
    }
}

} // imgproc

#endif // imgproc_scattered_interpolation_hpp_included_
