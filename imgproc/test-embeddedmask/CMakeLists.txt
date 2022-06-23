define_module(BINARY test-embeddedmask
  DEPENDS imgproc service
)

# load tool
set(dump-embeddedmask_SOURCES
  dump.cpp
  )

add_executable(dump-embeddedmask ${dump-embeddedmask_SOURCES})
target_link_libraries(dump-embeddedmask ${MODULE_LIBRARIES})
target_compile_definitions(dump-embeddedmask PRIVATE ${MODULE_DEFINITIONS}
  IMGPROC_VERSION="${MODULE_imgproc_VERSION}")
buildsys_binary(dump-embeddedmask)

# copy tool
set(copy-embeddedmask_SOURCES
  copy.cpp
  )

add_executable(copy-embeddedmask ${copy-embeddedmask_SOURCES})
target_link_libraries(copy-embeddedmask ${MODULE_LIBRARIES})
target_compile_definitions(copy-embeddedmask PRIVATE ${MODULE_DEFINITIONS}
  IMGPROC_VERSION="${MODULE_imgproc_VERSION}")
buildsys_binary(copy-embeddedmask)
