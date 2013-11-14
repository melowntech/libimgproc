set(imgproc_DEPENDS math utility dbglog Boost)

if(EXIF_FOUND)
  message(STATUS "imgproc: compiling in EXIF support")

  list(APPEND imgproc_DEPENDS EXIF)

  set(imgproc_EXIF_SOURCES
    exif.hpp exif.cpp)
else()
  message(STATUS "imgproc: compiling without EXIF support")
endif()

define_module(LIBRARY imgproc DEPENDS ${imgproc_DEPENDS})

set(imgproc_SOURCES
  color.cpp
  clahe.cpp
  rastermask.hpp
  rastermask.cpp
  georeferencing.hpp
  scanconversion.hpp
  scanconversion.cpp
)

add_library(imgproc STATIC ${imgproc_SOURCES} ${imgproc_EXIF_SOURCES})
buildsys_library(imgproc)
target_link_libraries(imgproc ${MODULE_LIBRARIES})

# test stuff
if (COMPILE_TESTS)
  add_subdirectory(test-tiff)
endif()
