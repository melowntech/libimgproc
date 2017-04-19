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
