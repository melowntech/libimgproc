define_module(LIBRARY imgproc DEPENDS math dbglog Boost)

set(imgproc_SOURCES
  color.cpp
  clahe.cpp
  rastermask.cpp
)

add_library(imgproc STATIC ${imgproc_SOURCES})
set_target_properties(imgproc PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(imgproc ${MODULE_LIBRARIES})
