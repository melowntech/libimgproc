define_module(BINARY test-tiff
  DEPENDS imgproc
)

set(test-tiff_SOURCES
  main.cpp
  )

add_executable(test-tiff ${test-tiff_SOURCES})
target_link_libraries(test-tiff ${MODULE_LIBRARIES})
buildsys_binary(test-tiff)
