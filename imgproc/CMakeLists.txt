set(imgproc_DEPENDS math utility dbglog Boost)

if(EXIF_FOUND)
  message(STATUS "improc: compiling in EXIF support")

  list(APPEND imgproc_DEPENDS EXIF)

  set(imgproc_EXIF_SOURCES
    exif.hpp exif.cpp)
endif()

define_module(LIBRARY imgproc DEPENDS ${imgproc_DEPENDS})

set(imgproc_SOURCES
  color.cpp
  clahe.cpp
  rastermask.hpp
  rastermask.cpp
  georeferencing.hpp
)

add_library(imgproc STATIC ${imgproc_SOURCES} ${imgproc_EXIF_SOURCES})
set_target_properties(imgproc PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(imgproc ${MODULE_LIBRARIES})
