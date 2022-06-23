define_module(BINARY test-tiff
  DEPENDS imgproc
)

# save tool
set(test-tiff-save_SOURCES
  save.cpp
  )

add_executable(test-tiff-save ${test-tiff-save_SOURCES})
target_link_libraries(test-tiff-save ${MODULE_LIBRARIES})
target_compile_definitions(test-tiff-save PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(test-tiff-save)

# load tool
set(test-tiff-load_SOURCES
  load.cpp
  )

add_executable(test-tiff-load ${test-tiff-load_SOURCES})
target_link_libraries(test-tiff-load ${MODULE_LIBRARIES})
target_compile_definitions(test-tiff-load PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(test-tiff-load)
