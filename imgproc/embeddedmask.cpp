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
