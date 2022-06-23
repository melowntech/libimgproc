define_module(BINARY imagesize
  DEPENDS imgproc
)

# load tool
set(convert_SOURCES
  convert.cpp
  )

add_executable(imgproc-convert ${convert_SOURCES})
target_link_libraries(imgproc-convert ${MODULE_LIBRARIES})
target_compile_definitions(imgproc-convert PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(imgproc-convert)
