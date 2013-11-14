#include <cstdlib>
#include <iostream>

#include "utility/filesystem.hpp"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << "tiff-file ifd data-file"
                  << std::endl;
        return EXIT_FAILURE;
    }
}
