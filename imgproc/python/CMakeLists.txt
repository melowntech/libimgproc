set(pyimgproc_DEPENDS
  imgproc=${MODULE_imgproc_VERSION}
  Boost_THREAD Boost_SYSTEM Boost_PYTHON PYTHONLIBS)
set(pyimgproc_SOURCES
  imgprocmodule.hpp
  imgprocmodule.cpp
  zbuffer.hpp
  )

if(NumPy_FOUND)
  message(STATUS "imgproc::python: compiling in numpy support")

  list(APPEND pyimgproc_DEPENDS NumPy OpenCV)
  list(APPEND pyimgproc_SOURCES zbuffer.cpp)
else()
  message(STATUS "imgproc::python: compiling without numpy support")
  list(APPEND pyimgproc_SOURCES zbuffer.dummy.cpp)
endif()

define_module(LIBRARY pyimgproc=${MODULE_imgproc_VERSION}
  DEPENDS
  imgproc=${MODULE_imgproc_VERSION}
  OpenCV Boost_THREAD Boost_SYSTEM Boost_PYTHON PYTHONLIBS)

add_library(pyimgproc STATIC ${pyimgproc_SOURCES})
buildsys_library(pyimgproc)
target_link_libraries(pyimgproc ${MODULE_LIBRARIES})
