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
#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"
#include "utility/streams.hpp"
#include "service/cmdline.hpp"

#include "imgproc/embeddedmask.hpp"
#include "imgproc/rastermask/cvmat.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

class Dumper : public service::Cmdline {
public:
    Dumper()
        : service::Cmdline("dump-embeddedmask", IMGPROC_VERSION)
        , raw_(false)
    {}

    virtual void configuration(po::options_description &cmdline
                               , po::options_description&
                               , po::positional_options_description &pd)
    {
        cmdline.add_options()
            ("input", po::value(&input_)->required()
             , "Input image file.")
            ("output", po::value(&output_)->required()
             , "Output mask file.")
            ("raw", "Dump raw mask.")
            ;

        pd.add("input", 1)
            .add("output", 1);
    }

    virtual void configure(const po::variables_map &vars) {
        raw_ = vars.count("raw");
    }

    virtual int run() {
        auto mask(imgproc::readEmbeddedMask(input_));
        if (raw_) {
            utility::ofstreambuf os(output_.string());
            mask.dump(os);
            os.close();
        } else {
            cv::imwrite(output_.string(), asCvMat(mask));
        }
        return EXIT_SUCCESS;
    }

private:
    fs::path input_;
    fs::path output_;
    bool raw_;
};


int main(int argc, char *argv[])
{
    dbglog::set_mask("ALL");
    return Dumper()(argc, argv);
}
