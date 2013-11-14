#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"
#include "imgproc/tiff.hpp"
#include "utility/streams.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << "tiff-file data-file"
                  << std::endl;
        return EXIT_FAILURE;
    }

    auto tiff(imgproc::tiff::openAppend(argv[1]));
    auto data(utility::read(argv[2]));

    auto dir(tiff.readDirectory());

    tiff.writeDirectory();

    LOG(info4) << "dir: " << dir;
}
