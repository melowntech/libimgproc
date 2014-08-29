#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "dbglog/dbglog.hpp"
#include "imgproc/embeddedmask.hpp"
#include "imgproc/rastermask/cvmat.hpp"

int main(int argc, char *argv[])
{
    dbglog::set_mask("ALL");
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << "image-file mask-file"
                  << std::endl;
        return EXIT_FAILURE;
    }

    auto mask(asCvMat(imgproc::readEmbeddedMask(argv[1])));

    imwrite(argv[2], mask);

    return EXIT_SUCCESS;
}
