#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"
#include "imgproc/bintiff.hpp"
#include "utility/streams.hpp"

void load(std::istream &is, std::string &data)
{
    data.clear();

    char tmp[1024];
    for (;;) {
        auto r(is.readsome(tmp, sizeof(tmp)));
        if (!r) { break; }
        data.append(tmp, r);
    }
}

int main(int argc, char *argv[])
{
    dbglog::set_mask("ALL");
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << "tiff-file data-file"
                  << std::endl;
        return EXIT_FAILURE;
    }

    auto tiff(imgproc::tiff::openRead(argv[1]));
    auto filename(argv[2]);

    std::string data;
    load(tiff.istream(filename), data);
    std::cout << data;
}
