/**
 * @file bitdept.hpp
 * @author Ladislav Horky <ladislav.horky@citationtech.net>
 *
 * Bit depth conversions
 */

#ifndef IMGPROC_BITDEPTH_HPP
#define IMGPROC_BITDEPTH_HPP

#include <opencv2/core/core.hpp>
#include "dbglog/dbglog.hpp"

namespace imgproc {

inline cv::Mat get8BitImage( const cv::Mat image ) {
    cv::Mat result8bit;
    
    if ( image.depth() == CV_16U ) {
        
        image.convertTo( result8bit, CV_8U, 0.00390625 );
    } else if ( image.depth() == CV_8U ) {
        
        result8bit = image.clone();
    } else {
        
        LOGTHROW(err2, std::runtime_error) 
                << "Only 16bit->8bit conversion implemented!";
    }
    
    return result8bit;
}

} // namespace imgproc

#endif // IMGPROC_COLOR_HPP
