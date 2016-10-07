//#include <opencv2/highgui.hpp>

#include "./inpaint.hpp"
#include "./scattered-interpolation.hpp"

namespace imgproc {

void jpegBlockInpaint(cv::Mat &img, const cv::Mat &mask,
                      int blkWidth, int blkHeight)
{
    //cv::imwrite("mask.png", mask);
    //cv::imwrite("before.png", img);

    imgproc::RasterMask blkMask(blkWidth, blkHeight);
    cv::Mat block(blkHeight, blkWidth, CV_32F);

    int nch = img.channels();

    for (int by = 0; by < img.rows; by += blkHeight)
    for (int bx = 0; bx < img.cols; bx += blkWidth)
    {
        int w = std::min(blkWidth,  img.cols - bx);
        int h = std::min(blkHeight, img.rows - by);

        bool full = true, empty = true;

        for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            bool m = mask.at<uchar>(by+y, bx+x);
            if (m) { empty = false; }
            else { full = false; }
            blkMask.set(x, y, m);
        }

        if (full) { continue; }

        if (!empty)
        {
            for (int ch = 0; ch < nch; ch++)
            {
                for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    block.at<float>(y, x) =
                        img.ptr<uchar>(by+y)[(bx+x)*nch + ch];
                }

                laplaceInterpolate(block, blkMask, 1e-3);

                for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    if (!blkMask.get(x, y)) {
                        img.ptr<uchar>(by+y)[(bx+x)*nch + ch] =
                            block.at<float>(y, x);
                    }
                }
            }
        }
        else // make sure empty block is black
        {
            for (int ch = 0; ch < nch; ch++)
            for (int y = 0; y < h; y++)
            for (int x = 0; x < w;  x++)
            {
                img.ptr<uchar>(by+y)[(bx+x)*nch + ch] = 0;
            }
        }
    }

    //cv::imwrite("inpaint.png", img);
}

} // namespace imgproc
