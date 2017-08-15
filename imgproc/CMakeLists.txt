# bump version here
set(imgproc_VERSION 1.21)

set(imgproc_DEPENDS math>=1.3 utility>=1.11 dbglog>=1.4 Boost)
set(imgproc_DEFINITIONS)

if(EXIF_FOUND)
  message(STATUS "imgproc: compiling in EXIF support")

  list(APPEND imgproc_DEPENDS EXIF)

  set(imgproc_EXIF_SOURCES
    exif.hpp exif.cpp)
else()
  message(STATUS "imgproc: compiling without EXIF support")
endif()

if(PNG_FOUND)
  message(STATUS "imgproc: compiling in PNG support")

  list(APPEND imgproc_DEPENDS PNG)
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_PNG=1)

  set(imgproc_PNG_SOURCES
    png.hpp png.cpp)
else()
  message(STATUS "imgproc: compiling without PNG support")
endif()

if(JPEG_FOUND)
  message(STATUS "imgproc: compiling in JPEG support")

  list(APPEND imgproc_DEPENDS JPEG)
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_JPEG=1)
  set(imgproc_JPEG_SOURCES
    jpeg.hpp jpeg.cpp)
else()
  message(STATUS "imgproc: compiling without JPEG support")
endif()

if(TIFF_FOUND)
  message(STATUS "imgproc: compiling in TIFF support")

  list(APPEND imgproc_DEPENDS TIFF)
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_TIFF=1)

  set(imgproc_TIFF_SOURCES
    tiff.hpp tiff.cpp
    bintiff.hpp bintiff.cpp)
else()
  message(STATUS "imgproc: compiling without TIFF support")
endif()

if(OpenCV_FOUND)
  message(STATUS "imgproc: compiling in OpenCV support")
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_OPENCV=1)

  list(APPEND imgproc_DEPENDS OpenCV)

  set(imgproc_OPENCV_SOURCES
    cvmat.hpp
    rastermask/cvmat.hpp rastermask/cvmat.cpp
    rastermask/transform.hpp rastermask/transform.cpp
    readimage.hpp readimage.cpp
    findrects.hpp detail/findrects.impl.hpp
    uvpack.hpp uvpack.cpp
    clahe.cpp
    spectral_analysis.hpp spectral_analysis.cpp
    scanconversion.hpp scanconversion.cpp
    )
else()
  message(STATUS "imgproc: compiling without OpenCV support")
endif()

if(EIGEN3_FOUND)
  message(STATUS "imgproc: compiling in Eigen3 support")
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_EIGEN3=1)

  list(APPEND imgproc_DEPENDS EIGEN3)

  set(imgproc_EIGEN3_SOURCES)
else()
  message(STATUS "imgproc: compiling without Eigen3 support")
endif()

if(OpenCV_FOUND AND EIGEN3_FOUND)
  # inpaint and scatteed interpolation depend on both OpenCV and Eigen3
  list(APPEND imgproc_EIGEN3_SOURCES
    scattered-interpolation.hpp scattered-interpolation.cpp
    inpaint.hpp inpaint.cpp)
endif()


if(GIF_FOUND AND OpenCV_FOUND)
  message(STATUS "imgproc: compiling in GIF support")
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_GIF=1)

  list(APPEND imgproc_DEPENDS GIF)

  set(imgproc_GIF_SOURCES
    gif.hpp gif.cpp)
else()
  message(STATUS "imgproc: compiling without GIF support")
endif()

define_module(LIBRARY imgproc=${imgproc_VERSION}
  DEPENDS ${imgproc_DEPENDS}
  DEFINITIONS ${imgproc_DEFINITIONS})

set(imgproc_SOURCES
  color.cpp
  bitdepth.hpp
  rastermask.hpp rastermask/bitfield.hpp rastermask/quadtree.hpp
  rastermask/bitfield.cpp rastermask/quadtree.cpp
  rastermask/mappedqtree.cpp rastermask/mappedqtree.cpp

  georeferencing.hpp
  gil-float-image.hpp
  crop.hpp

  morphology.hpp

  embeddedmask.hpp embeddedmask.cpp

  const-raster.hpp
  filtering.hpp reconstruct.hpp

  jp2.hpp jp2.cpp

  texturing.hpp texturing.cpp

  colormap.hpp

  contours.hpp contours.cpp

  svg.hpp
  )

add_library(imgproc STATIC ${imgproc_SOURCES}
  ${imgproc_EXIF_SOURCES}
  ${imgproc_TIFF_SOURCES}
  ${imgproc_OPENCV_SOURCES}
  ${imgproc_EIGEN3_SOURCES}
  ${imgproc_GIF_SOURCES}
  ${imgproc_PNG_SOURCES}
  ${imgproc_JPEG_SOURCES}
  )
buildsys_library(imgproc)
target_link_libraries(imgproc ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(imgproc ${MODULE_DEFINITIONS})

# test stuff
if (COMPILE_TESTS)
  add_subdirectory(test-tiff)
  add_subdirectory(test-embeddedmask)
endif()
