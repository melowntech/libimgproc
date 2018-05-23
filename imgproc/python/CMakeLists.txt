define_module(LIBRARY pyimgproc=${MODULE_imgproc_VERSION}
  DEPENDS
  imgproc=${MODULE_imgproc_VERSION}
  Boost_THREAD Boost_SYSTEM Boost_PYTHON PYTHONLIBS)

set(pyimgproc_SOURCES
  imgprocmodule.cpp
  )

add_library(pyimgproc STATIC ${pyimgproc_SOURCES})
buildsys_library(pyimgproc)
target_link_libraries(pyimgproc ${MODULE_LIBRARIES})
