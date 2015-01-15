#include <opencv2/highgui/highgui.hpp>

#include "./readimage.hpp"

#ifdef IMGPROC_HAS_GIF
#  include "./gif.hpp"
#endif

namespace imgproc {

cv::Mat readImage(const void *data, std::size_t size)
{
    auto image(cv::imdecode({data, int(size)}, CV_LOAD_IMAGE_COLOR));

#ifdef IMGPROC_HAS_GIF
    if (!image.data) {
        // try gif
        try {
            image = imgproc::readGif(data, size);
        } catch (const std::runtime_error &e) {
        }
    }
#endif

    // TODO: try tiff
    return image;
}

cv::Mat readImage(const boost::filesystem::path &path)
{
    auto image(cv::imread(path.string(), CV_LOAD_IMAGE_COLOR));

#ifdef IMGPROC_HAS_GIF
    if (!image.data) {
        // try gif
        try {
            image = imgproc::readGif(path);
        } catch (const std::runtime_error &e) {
        }
    }
#endif

    // TODO: try tiff
    return image;
}

} // namespace imgproc
