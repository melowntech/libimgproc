#include <vector>

#include <Eigen/Sparse>

#include "dbglog/dbglog.hpp"

#include "./scattered-interpolation.hpp"

namespace imgproc {

typedef Eigen::SparseMatrix<double> SparseMatrix;
typedef Eigen::Triplet<double> Triplet;


void laplaceInterpolate(cv::Mat &data, const imgproc::RasterMask &mask)
{
    int w = data.cols, h = data.rows;
    int n = w * h; // # of unknowns

    // assemble the linear system
    LOG(info1) << "Assembling " << n << "x" << n << " sparse system.";

    std::vector<Triplet> coefs;
    Eigen::VectorXd rhs(n);

    for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
    {
        int k = i*w + j; // index of the unknown
        coefs.emplace_back(k, k, 1.0);

        if (mask.get(j, i)) // given point
        {
            rhs(k) = data.at<float>(i, j);
        }
        else // free point
        {
            if (i > 0 && i < h-1 && j > 0 && j < w-1) // 4 neighbors
            {
                coefs.emplace_back(k, k-1, -0.25);
                coefs.emplace_back(k, k+1, -0.25);
                coefs.emplace_back(k, k-w, -0.25);
                coefs.emplace_back(k, k+w, -0.25);
            }
            else  // 2 neighbors
            {
                int n1, n2;
                if (i > 0 && i < h-1) { // left or right edge
                    n1 = -w;
                    n2 = w;
                }
                else if (j > 0 && j < w-1) { // top or bottom edge, 2 neighbors
                    n1 = -1;
                    n2 = 1;
                }
                else { // corners
                    n1 = i ? -w : w;
                    n2 = j ? -1 : 1;
                }
                coefs.emplace_back(k, k+n1, -0.5);
                coefs.emplace_back(k, k+n2, -0.5);
            }

            rhs(k) = 0.0;
        }
    }

    SparseMatrix mat(n, n);
    mat.setFromTriplets(coefs.begin(), coefs.end());

    LOG(debug) << "mat = \n" << mat;
    LOG(debug) << "rhs = \n" << rhs;

    // solve the system
    LOG(info1) << "Solving system.";
    Eigen::BiCGSTAB<SparseMatrix, Eigen::IncompleteLUT<double> > solver(mat);
    Eigen::VectorXd sln(solver.solve(rhs));

    LOG(info1) << "#iterations: " << solver.iterations();
    LOG(info1) << "estimated error: " << solver.error();

    // return solution
    for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
    {
        data.at<float>(i, j) = sln(i*w + j);
    }
}


} // namespace imgproc
