define_module(BINARY test-embeddedmask
  DEPENDS imgproc
)

# load tool
set(dump-embeddedmask_SOURCES
  dump.cpp
  )

add_executable(dump-embeddedmask ${dump-embeddedmask_SOURCES})
target_link_libraries(dump-embeddedmask ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(dump-embeddedmask PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(dump-embeddedmask)
