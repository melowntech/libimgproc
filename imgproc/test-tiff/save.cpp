#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"
#include "imgproc/bintiff.hpp"
#include "utility/streams.hpp"

void dump(std::ostream &os, const std::string &data)
{
    os.write(data.data(), data.size());
}

int main(int argc, char *argv[])
{
    dbglog::set_mask("ALL");
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << "tiff-file data-file"
                  << std::endl;
        return EXIT_FAILURE;
    }

    auto tiff(imgproc::tiff::openAppend(argv[1]));
    auto filename(argv[2]);
    auto data(utility::read(filename));

    dump(tiff.ostream(filename), data);
}
