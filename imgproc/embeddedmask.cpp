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

#include "dbglog/dbglog.hpp"

#include "utility/zip.hpp"

#include "./embeddedmask.hpp"
#include "./bintiff.hpp"
#include "./imagesize.hpp"

namespace bio = boost::iostreams;
namespace fs = boost::filesystem;

namespace imgproc {

namespace {
const std::string BintiffMaskName("validity-mask.bin");
const fs::path ZipMaskName("validity-mask.bin");
const fs::path ZipMaskNameFull("/validity-mask.bin");

#if IMGPROC_HAS_TIFF
quadtree::RasterMask maskFromTiff(const fs::path &imagePath)
{
    LOG(info1) << "Loading mask from " << imagePath.string()
               << "/" << BintiffMaskName << ".";

    quadtree::RasterMask mask;
    auto tiff(imgproc::tiff::openRead(imagePath));
    mask.load(tiff.istream(BintiffMaskName));
    return mask;
}

void maskToTiff(const fs::path &imagePath
                , const quadtree::RasterMask &mask)
{
    LOG(info1) << "Saving mask to " << imagePath.string()
               << "/" << BintiffMaskName << ".";

    // try tiff
    auto tiff(imgproc::tiff::openAppend(imagePath));
    mask.dump(tiff.ostream(BintiffMaskName));
    return;
}

#else
quadtree::RasterMask maskFromTiff(const fs::path &imagePath)
{
    LOGTHROW(err1, std::runtime_error)
        << "Cannot load raster mask from " << imagePath
        << ": TIFF support not compiled in.";
}

void maskToTiff(const fs::path &imagePath, const quadtree::RasterMask&)
{
    LOGTHROW(err1, std::runtime_error)
        << "Cannot save raster mask to " << imagePath
        << ": TIFF support not compiled in.";
}

#endif

quadtree::RasterMask maskFromZip(const fs::path &imagePath)
{
    if (!utility::zip::Reader::check(imagePath)) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot load raster mask from " << imagePath
            << ": does not contain a ZIP archive.";
    }

    // open zip archive and try to find file with mask
    utility::zip::Reader zip(imagePath);
    auto zipIndex(zip.find(ZipMaskNameFull));

    quadtree::RasterMask mask;
    {
        // load mask from embedded file
        bio::filtering_istream fis;
        zip.plug(zipIndex, fis);
        mask.load(fis);
    }
    return mask;
}

void maskToZip(const fs::path &imagePath, const quadtree::RasterMask &mask)
{
    // TODO: truncate to original size if anything crashes

    utility::zip::Writer zip(imagePath, utility::zip::Embed);

    {
        auto os(zip.ostream(ZipMaskName, utility::zip::Compression::deflate));
        mask.dump(os->get());
        os->close();
    }

    zip.close();
}

} // namespace

void writeEmbeddedMask(const fs::path &imagePath
                       , const quadtree::RasterMask &mask)
{
    const auto type(imageMimeType(imagePath));
    if (type.empty()) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot save raster mask to " << imagePath
            << ": Unsupported image type.";
    }

    if (type == "image/tiff") {
        return maskToTiff(imagePath, mask);
    }

    return maskToZip(imagePath, mask);
}

quadtree::RasterMask readEmbeddedMask(const fs::path &imagePath)
{
    const auto type(imageMimeType(imagePath));
    if (type.empty()) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot load raster mask from " << imagePath
            << ": Unsupported image type.";
    }

    if (type == "image/tiff") {
        return maskFromTiff(imagePath);
    }

    return maskFromZip(imagePath);
}

boost::optional<quadtree::RasterMask>
readEmbeddedMask(const fs::path &imagePath, const std::nothrow_t&)
{
    try {
        return readEmbeddedMask(imagePath);
    } catch (std::exception) {}

    return boost::none;
}

} // namespace imgproc
