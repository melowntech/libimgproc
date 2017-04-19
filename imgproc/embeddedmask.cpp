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

#include "./embeddedmask.hpp"
#include "./bintiff.hpp"

namespace fs = boost::filesystem;

namespace imgproc {

namespace {
    const std::string BintiffMaskName("validity-mask.bin");
}

void writeEmbeddedMask(const fs::path &imagePath
                       , const quadtree::RasterMask &mask)
{
    // TODO: try/catch if support for other types is added

    LOG(info1) << "Saving mask to " << imagePath.string()
               << "/" << BintiffMaskName << ".";

#if IMGPROC_HAS_TIFF
    // try tiff
    auto tiff(imgproc::tiff::openAppend(imagePath));
    mask.dump(tiff.ostream(BintiffMaskName));
    return;
#endif

    LOGTHROW(err1, std::runtime_error)
        << "Cannot save raster mask into " << imagePath
        << ": Unsupported image type.";
    (void) mask;
}

quadtree::RasterMask readEmbeddedMask(const fs::path &imagePath)
{
    LOG(info1) << "Loading mask from " << imagePath.string()
               << "/" << BintiffMaskName << ".";
#if IMGPROC_HAS_TIFF
    {
        quadtree::RasterMask mask;
        auto tiff(imgproc::tiff::openRead(imagePath));
        mask.load(tiff.istream(BintiffMaskName));
        return mask;
    }
#endif

    LOGTHROW(err1, std::runtime_error)
        << "Cannot load raster mask from " << imagePath
        << ": Unsupported image type.";
    throw;
}

boost::optional<quadtree::RasterMask>
readEmbeddedMask(const fs::path &imagePath, std::nothrow_t)
{
    try {
        return readEmbeddedMask(imagePath);
    } catch (std::exception) {}

    return boost::none;
}

} // namespace imgproc
