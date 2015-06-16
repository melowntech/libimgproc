#include <arpa/inet.h>

#include <fstream>

#include "dbglog/dbglog.hpp"

#include "utility/binaryio.hpp"

#include "./error.hpp"
#include "./jp2.hpp"

namespace imgproc {

namespace bio = utility::binaryio;

namespace jp2 {
    const std::uint32_t Signature(0x6a502020);

    const std::uint8_t Magic[4] = { 0x0d, 0x0a, 0x87, 0x0a };

    const std::uint32_t FileType(0x66747970);
    const std::uint32_t Header(0x6a703268);
    const std::uint32_t ImageHader(0x69686472);

    struct Box {
        std::uint32_t size;
        std::uint32_t type;
        std::vector<std::uint8_t> data;
    };

    Box readBox(std::istream &is) {
        Box box;
        bio::read(is, box.size);
        bio::read(is, box.type);
        box.size = ntohl(box.size);
        box.type = ntohl(box.type);

        // skip reading data of super header
        if (box.type == Header) {
            return box;
        }
        box.data.resize(box.size - 8);
        bio::read(is, box.data.data(), box.data.size());
        return box;
    }

    std::uint32_t get32(const void *raw) {
        return ntohl(*static_cast<const std::uint32_t*>(raw));
    }

    std::uint32_t get32(const Box &box, unsigned int index) {
        return get32(&box.data[index]);
    }
}

math::Size2 jp2Size(const boost::filesystem::path &path)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);

    {
        auto signature(jp2::readBox(f));
        if (signature.type != jp2::Signature) {
            LOGTHROW(err1, Error)
                << "Not a JP2 file: " << path << ": expected signature box.";
        }

        if (!std::equal(signature.data.begin(), signature.data.end()
                        , jp2::Magic))
        {
            LOGTHROW(err1, Error)
                << "Not a JP2 file: " << path << ": invalid magic.";
        }
    }

    if (jp2::readBox(f).type != jp2::FileType) {
        LOGTHROW(err1, Error)
            << "Not a JP2 file: " << path << ": expected file type box.";
    }

    if (jp2::readBox(f).type != jp2::Header) {
        LOGTHROW(err1, Error)
            << "Not a JP2 file: " << path << ": expected header box.";
    }

    // search for image header box
    for (int leftBoxes(10); leftBoxes; --leftBoxes) {
        auto box(jp2::readBox(f));
        if (box.type == jp2::ImageHader) {
            return { int(get32(box, 4)),  int(get32(box, 0)) };
        }
    }

    LOGTHROW(err1, Error)
        << "Not a JP2 file: " << path << ": unable to find image header.";
    throw;
}

} // namespace imgproc
