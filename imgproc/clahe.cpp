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
/*
 * clahe.cpp
 */

#include <stdexcept>


#include "clahe.hpp"

#include <dbglog/dbglog.hpp>
#include <math/math_all.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include "detail/clahe.hpp"

namespace imgproc {

void CLAHE( const cv::Mat & src, cv::Mat & dst, const int regionSize,
            float clipLimit ) {

    if (!src.cols || !src.rows) {
        LOGTHROW(err2, std::runtime_error)
            << "CLAHE: Empty input image.";
    }

    cv::Mat srcsc, rawscn, dstsc;
    std::vector<cv::Mat> srcsplit;
    
    if ( src.type() != CV_8UC1 && src.type() != CV_16UC1
        && src.type() != CV_8UC3 && src.type() != CV_16UC3 )
        LOGTHROW( err2, std::runtime_error ) << "CLAHE does not support "
            " image type " << src.type() << ".";

    // convert channels
    if ( src.type() == CV_8UC1 || src.type() == CV_16UC1 ) {
        
        srcsc = src;
    }

    if ( src.type() == CV_8UC3 || src.type() == CV_16UC3 ) {

        cv::Mat srcluv;
        
        cvtColor( src, srcluv, cv::COLOR_RGB2YCrCb );

        split( srcluv, srcsplit );

        srcsc = srcsplit[0];        
    }

    // convert dimensions
    int divx = (int) ceil( (float) srcsc.cols / regionSize );
    int divy = (int) ceil( (float) srcsc.rows / regionSize );
    
    rawscn.create( divy * regionSize, divx * regionSize, srcsc.type() );
    rawscn.setTo(cv::Scalar::all(0));
    
    srcsc.copyTo(
        rawscn( cv::Rect( 0, 0, srcsc.cols, srcsc.rows ) ) );
    
    // clahe
    int retval( -1 );
    
    if ( src.depth() == CV_8U ) {
        
        retval = detail::CLAHE<unsigned char>(
            reinterpret_cast<unsigned char *>( rawscn.data ),
            rawscn.cols, rawscn.rows,
            0, 0xff, divx, divy, 0x100, clipLimit );
    }

    if ( src.depth() == CV_16U ) {

        retval = detail::CLAHE<unsigned short>(
            reinterpret_cast<unsigned short *>( rawscn.data ),
            rawscn.cols, rawscn.rows,
            0, 0xffff, divx, divy, std::min( 0x10000, math::sqr( regionSize ) ), clipLimit );
    }

    if ( retval != 0 ) {

        LOGTHROW( err2, std::runtime_error ) << "CLAHE returned error code "
            << retval << ". See imgproc/detail/clahe.hpp for details.";
    }
    
    // convert dimensions
    dstsc.create( srcsc.rows, srcsc.cols, srcsc.type() );
    rawscn.rowRange( 0, srcsc.rows ).colRange( 0, srcsc.cols ).copyTo( dstsc );
    
    // convert channels
    if ( src.type() == CV_8UC1 || src.type() == CV_16UC1 ) {

        dst = dstsc;
    }

    if ( src.type() == CV_8UC3 || src.type() == CV_16UC3 ) {

        cv::Mat dstluv;
        
        srcsplit[0] = dstsc;

        merge( srcsplit, dstluv );
    
        cvtColor( dstluv, dst, cv::COLOR_YCrCb2RGB );
    }
    
    // done
}



} // namespace imgproc
