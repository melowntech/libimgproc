set(pyimgproc_DEPENDS
  imgproc=${MODULE_imgproc_VERSION}
  pysupport>=1.4
  Boost_THREAD Boost_SYSTEM)

set(pyimgproc_SOURCES
  imgprocmodule.hpp
  imgprocmodule.cpp
  zbuffer.hpp
  )

set(pyimgproc_DEFINITIONS)

if(NumPy_FOUND)
  message(STATUS "imgproc::python: compiling in numpy support")

  list(APPEND pyimgproc_DEPENDS NumPy OpenCV)
  list(APPEND pyimgproc_SOURCES
    zbuffer.cpp numpy.hpp numpy.cpp
    detail/numpy.hpp detail/numpy.cpp)
  list(APPEND pyimgproc_DEFINITIONS PYIMGPROC_HAS_NUMPY=1)
else()
  message(STATUS "imgproc::python: compiling without numpy support")
  list(APPEND pyimgproc_SOURCES zbuffer.dummy.cpp numpy.dummy.cpp)
endif()

if(MODULE_geometry_FOUND)
  message(STATUS "imgproc::python: compiling in geometry support")

  list(APPEND pyimgproc_DEPENDS geometry)
  list(APPEND pyimgproc_DEFINITIONS PYIMGPROC_HAS_GEOMETRY=1)
else()
  message(STATUS "imgproc::python: compiling without geometry support")
endif()

define_module(LIBRARY pyimgproc=${MODULE_imgproc_VERSION}
  DEPENDS
  imgproc=${MODULE_imgproc_VERSION}
  Boost_THREAD Boost_SYSTEM Boost_PYTHON PYTHONLIBS
  DEFINITIONS ${pyimgproc_DEFINITIONS})

add_library(pyimgproc STATIC ${pyimgproc_SOURCES})
buildsys_library(pyimgproc)
target_link_libraries(pyimgproc ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(pyimgproc ${MODULE_DEFINITIONS})
