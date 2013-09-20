#include "exif.hpp"

namespace imgproc { namespace exif {

Exif::Handle Exif::open(const boost::filesystem::path &path)
{
    auto ed(::exif_data_new_from_file(path.string().c_str()));
    if (!ed) {
        LOGTHROW(err2, std::runtime_error)
            << "Failed to read exif from file " << path << ".";
    }

    return {ed, [](::ExifData *ed) {
            if (ed) { ::exif_data_unref(ed); }
        }};
}

Exif::Entry Exif::getEntry(ExifTag tag, ExifIfd ifd) const
{
    auto e((ifd == EXIF_IFD_COUNT)
           ? exif_data_get_entry(ed_, tag)
           : exif_content_get_entry(ed_->ifd[ifd], tag));
    if (!e) {
        if (ifd == EXIF_IFD_COUNT) {
            LOGTHROW(warn1, NoSuchTag)
                << "No tag <" << tag << "/whole> found in file " << path_
                << ".";
        } else {
            LOGTHROW(warn1, NoSuchTag)
                << "No tag <" << tag << "/" << ::exif_ifd_get_name(ifd)
                << "> found in file "
                << path_ << ".";
        }
    }

    return Entry(ed_, e);
}

Rational Exif::getFPResolutionUnit(ExifIfd ifd) const
{
    switch (auto value = getEntry(EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT, ifd)
            .as<short>())
    {
    case 1: return {1};
    case 2: return inch;
    case 3: return centimeter;
    default:
        LOGTHROW(err1, InvalidValue)
            << "Invalid value of Focal plane resolution unit: <"
            << value << "> in file " << path_ << ".";
    }
    throw;
}

Orientation Exif::getOrientation(ExifIfd ifd) const
{
    switch (auto value = getEntry(EXIF_TAG_ORIENTATION, ifd).as<short>()) {
    case 1: return Orientation::top_left;
    case 2: return Orientation::top_right;
    case 3: return Orientation::bottom_right;
    case 4: return Orientation::bottom_left;
    case 5: return Orientation::left_top;
    case 6: return Orientation::right_top;
    case 7: return Orientation::right_bottom;
    case 8: return Orientation::left_bottom;
    default:
        LOGTHROW(warn1, InvalidValue)
            << "Invalid value of Orientation: <" << value << "> in file "
            << path_ << ".";
    }
    throw;
}

} } // namespace imgproc::exif
