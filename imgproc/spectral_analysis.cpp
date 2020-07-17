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
 * @file spectral_analysis.hpp
 * @author Ladislav Horky <ladislav.horky@citationtech.net>
 *
 * Spectral analysis functions
 */

#ifndef imgproc_spectral_analysis_hpp_included_
#define imgproc_spectral_analysis_hpp_included_

#include <opencv2/imgproc/imgproc.hpp>

#include "dbglog/dbglog.hpp"

#include "cvcompat.hpp"

namespace imgproc {

namespace {

void PrintMatrix(std::stringstream &ss, const cv::Mat &m, const int prec){
    ss << std::setprecision(prec) << std::setw(8) << std::fixed;
    int i = 0;
    ss << "\nDCT histogram matrix:\n";
    for(auto el = m.begin<float>(); el < m.end<float>(); el++,i++){
        if(i && !(i % m.cols)) ss << "\n";
        ss << std::setw(8) << *el;
    }
    ss << "\n\n";
}

} // namespace

void effectiveScale(const cv::Mat & img, float &hscale, float &vscale
                    , float threshold)
{
    
    if (!img.cols || !img.rows) {
        LOGTHROW(err2, std::runtime_error)
            << "EffectiveScale: Empty input image.";
    }

    cv::Mat src;
    
    if ( img.type() != CV_8UC1 && img.type() != CV_16UC1
        && img.type() != CV_8UC3 && img.type() != CV_16UC3 )
        LOGTHROW( err3, std::runtime_error ) << "EffectiveScale does not support "
            " image type " << img.type() << ".";

    // convert channels
    if ( img.type() == CV_8UC1 || img.type() == CV_16UC1 ) {     
        src = img;
    }

    if ( img.type() == CV_8UC3 || img.type() == CV_16UC3 ) {   
        cvtColor( img, src, IMGPROC_CVT_COLOR(RGB2GRAY));
    }
    
    src.convertTo(src,CV_32F);
    //stretch to 0-255
    auto maxel = std::max_element(src.begin<float>(),src.end<float>());
    auto minel = std::min_element(src.begin<float>(),src.end<float>());
    
    //std::cout << *minel << " - " << *maxel << "\n";
    
    for(auto el = src.begin<float>();el < src.end<float>();el++){
        *el = (*el-*minel)*255/(*maxel-*minel);
    }
    
    //compute std of pixels
    double x = 0, x2 = 0;
    for(auto el = src.begin<float>();el < src.end<float>();el++){
        x += *el;
        x2 += (*el) * (*el);
    }
    double cap = src.cols*src.rows;
    LOG(info1) << x << ", " << x2 << ", cap: "<< cap;
    double std = sqrt(x2/cap - (x*x)/(cap*cap));
    
    
    //compute 8x8 cumulative histogram from dct blocks
    const int dctSize = 8;
    const float tr = std/5;         //experimental threashold
    src = src(cv::Rect(0, 0, (src.cols/dctSize)*dctSize, (src.rows/dctSize)*dctSize)); //shrink image to multiple of dctSize
    
    cv::Mat cumulHist = cv::Mat::zeros(dctSize,dctSize,CV_32F), 
            tmpDct = cv::Mat::zeros(dctSize,dctSize,CV_32F);
    
    for(int i=0; i<src.cols; i+=dctSize){
        for(int j=0; j<src.rows; j+=dctSize){
            //compute dct of 8x8 pixel block
            cv::dct(src(cv::Rect(i,j,dctSize,dctSize)),tmpDct);
            
            auto histIt(cumulHist.begin<float>());
            //add to cumulHist if higher than tr
            for(auto el = tmpDct.begin<float>(); el<tmpDct.end<float>(); el++,histIt++){
                if(abs(*el) > tr) (*histIt)++;
            }
        }
    }
    
    //compute row and col sums
    cv::Mat rowSum = cumulHist * cv::Mat::ones(cumulHist.cols,1,CV_32F);
    cv::Mat colSum = cv::Mat::ones(1,cumulHist.rows,CV_32F) * cumulHist;
    
    //norm to 1st (highest) value
    rowSum /= rowSum.at<float>(0,0);
    colSum /= colSum.at<float>(0,0);
    
    //find where it cuts the threshold for the last time
    auto it = rowSum.end<float>()-1;
    int i;
    for(i = dctSize-1; *it < threshold; it--,i--);
    //numerator = where exactly it cuts threshold
    if(i == dctSize-1){ vscale = 1.0; }
    else{ vscale = (i+ (threshold- *it) / (*(it+1) - *it))/(dctSize-1); }
    
    it = colSum.end<float>()-1;
    for(i = dctSize-1; *it < threshold; it--,i--);
    //numerator = where exactly it cuts threshold
    if(i == dctSize-1){ hscale = 1.0; }
    else{ hscale = (i+ (threshold- *it) / (*(it+1) - *it))/(dctSize-1); }
    
    //Print cumulHit to log
    cumulHist /= cumulHist.at<float>(0,0);
    std::stringstream ss;
    PrintMatrix(ss,cumulHist,5);
    std::string histStr(ss.str());
    LOG(info1) << histStr << "DCT threshold: " << tr;
}

} // namespace imgproc

#endif //imgproc_spectral_analysis_hpp_included_
