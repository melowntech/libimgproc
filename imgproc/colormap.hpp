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
 * @file colormap.hpp
 * @author Jakub Cerveny <jakub.cerveny@melown.com>
 *
 * Matlab-style colormap for data visualization purposes.
 */

#ifndef imgproc_colormap_included_hpp_
#define imgproc_colormap_included_hpp_

namespace imgproc {

/** The infamous "jet" color palette from Matlab.
 */
template<typename T>
class MatlabColorMap
{
public:
    static T red(T value) {
        return base(value - 0.5);
    }
    static T green(T value) {
        return base(value);
    }
    static T blue(T value) {
        return base(value + 0.5);
    }

    template<typename Vector>
    static Vector rgb(T value) {
        return {red(value), green(value), blue(value)};
    }
    template<typename Vector>
    static Vector bgr(T value) {
        return {blue(value), green(value), red(value)};
    }

protected:
    static T interpolate(T val, T y0, T x0, T y1, T x1) {
        return (val-x0)*(y1-y0)/(x1-x0) + y0;
    }

    static T base(T val) {
        if (val <= -0.75) {
            return 0;
        }
        else if (val <= -0.25) {
            return interpolate(val, 0.0, -0.75, 1.0, -0.25);
        }
        else if (val <= 0.25) {
            return 1.0;
        }
        else if (val <= 0.75) {
            return interpolate(val, 1.0, 0.25, 0.0, 0.75);
        }
        else {
            return 0.0;
        }
    }
};

} // namespace imgproc

#endif // imgproc_colormap_included_hpp_
