set(imgproc_DEPENDS math utility dbglog Boost)
set(imgproc_DEFINITIONS)

if(EXIF_FOUND)
  message(STATUS "imgproc: compiling in EXIF support")

  list(APPEND imgproc_DEPENDS EXIF)

  set(imgproc_EXIF_SOURCES
    exif.hpp exif.cpp)
else()
  message(STATUS "imgproc: compiling without EXIF support")
endif()

if(TIFF_FOUND)
  message(STATUS "imgproc: compiling in TIFF support")

  list(APPEND imgproc_DEPENDS TIFF)
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_TIFF=1)

  set(imgproc_TIFF_SOURCES
    bintiff.hpp bintiff.cpp)
else()
  message(STATUS "imgproc: compiling without TIFF support")
endif()

if(OpenCV_FOUND)
  message(STATUS "imgproc: compiling in OpenCV support")
  list(APPEND imgproc_DEFINITIONS IMGPROC_HAS_OPENCV=1)

  list(APPEND imgproc_DEPENDS OpenCV)

  set(imgproc_OPENCV_SOURCES
    cvmat.hpp rastermask/cvmat.hpp rastermask/cvmat.cpp
    readimage.hpp readimage.cpp
    findrects.hpp detail/findrects.hpp
    )
else()
  message(STATUS "imgproc: compiling without OpenCV support")
endif()

if(EIGEN3_FOUND)
  message(STATUS "imgproc: compiling in Eigen3 support")

  list(APPEND imgproc_DEPENDS EIGEN3)

  set(imgproc_EIGEN3_SOURCES
    scattered-interpolation.hpp scattered-interpolation.cpp)
else()
  message(STATUS "imgproc: compiling without Eigen3 support")
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

define_module(LIBRARY imgproc
  DEPENDS ${imgproc_DEPENDS}
  DEFINITIONS ${imgproc_DEFINITIONS})

set(imgproc_SOURCES
  color.cpp
  bitdepth.hpp
  clahe.cpp
  spectral_analysis.hpp spectral_analysis.cpp
  rastermask.hpp rastermask/bitfield.hpp rastermask/quadtree.hpp
  rastermask/bitfield.cpp rastermask/quadtree.cpp

  georeferencing.hpp
  scanconversion.hpp scanconversion.cpp
  gil-float-image.hpp
  crop.hpp

  morphology.hpp

  embeddedmask.hpp embeddedmask.cpp
)

add_library(imgproc STATIC ${imgproc_SOURCES}
  ${imgproc_EXIF_SOURCES}
  ${imgproc_TIFF_SOURCES}
  ${imgproc_OPENCV_SOURCES}
  ${imgproc_EIGEN3_SOURCES}
  ${imgproc_GIF_SOURCES}
  )
buildsys_library(imgproc)
target_link_libraries(imgproc ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(imgproc ${MODULE_DEFINITIONS})

# test stuff
if (COMPILE_TESTS)
  add_subdirectory(test-tiff)
  add_subdirectory(test-embeddedmask)
endif()
