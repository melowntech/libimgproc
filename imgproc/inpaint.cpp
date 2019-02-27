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
//#include <opencv2/highgui.hpp>

#include "inpaint.hpp"
#include "scattered-interpolation.hpp"

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
