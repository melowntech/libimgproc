/*
 * clahe.cpp
 */

#ifndef IMGPROC_CLAHE_HPP
#define IMGPROC_CLAHE_HPP


#include "clahe.hpp"

#include <dbglog/dbglog.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include "detail/clahe.hpp"


namespace imgproc {

void CLAHE( const cv::Mat & src, cv::Mat & dst, const int regionSize,
            float clipLimit = -1.0 ) {

    cv::Mat srcsc, rawscn, dstsc;
    std::vector<cv::Mat> srcsplit;
    
    if ( src.type() != CV_8UC1 && src.type() != CV_16UC1
        && src.type() != CV_8UC3 && src.type() != CV_16UC3 )
        LOGTHROW( err3, std::runtime_error ) << "CLAHE does not support "
            " image type " << src.type() << ".";

    // convert channels
    if ( src.type() == CV_8UC1 || src.type() == CV_16UC1 ) {
        
        srcsc = src;
    }

    if ( src.type() == CV_8UC3 || src.type() == CV_16UC3 ) {

        cv::Mat srcluv;
        
        cvtColor( src, srcluv, CV_RGB2YCrCb );

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
            0, 0xffff, divx, divy, 0x10000, clipLimit );
    }

    if ( retval != 0 ) {

        LOGTHROW( err3, std::runtime_error ) << "CLAHE returned error code "
            << "retval. See imgproc/detail/clahe.hpp for details.";
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
    
        cvtColor( dstluv, dst, CV_YCrCb2RGB );
    }
    
    // done
}



} // namespace imgproc

#endif // IMGPROC_CLAHE_HPP