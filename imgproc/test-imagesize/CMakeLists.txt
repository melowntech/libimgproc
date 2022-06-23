define_module(BINARY imagesize
  DEPENDS imgproc
)

# load tool
set(imagesize_SOURCES
  imagesize.cpp
  )

add_executable(imgproc-imagesize ${imagesize_SOURCES})
target_link_libraries(imgproc-imagesize ${MODULE_LIBRARIES})
target_compile_definitions(imgproc-imagesize PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(imgproc-imagesize)
