
find_package(PythonLibs 2 REQUIRED)

find_package(Boost COMPONENTS python REQUIRED)


include_directories(
  ${CMAKE_SOURCE_DIR} ${BULLET_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS}
  )

link_directories(
  ${PYTHON_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS}
  )

add_definitions(
  ${BOOST_DEFINITIONS}
  )
if(WIN32 AND Boost_USE_STATIC_LIBS)
  add_definitions(-DBOOST_PYTHON_STATIC_LIB)
endif()

set(SIMULOTTER_PYTHON_LIBS
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )


set(python_src
  display.cpp galipeur.cpp main.cpp maths.cpp object.cpp physics.cpp robot.cpp
  sensors.cpp utils.cpp
  )
foreach(m ${SIMULOTTER_ENABLED_MODULES})
  list(APPEND python_src ${m}.cpp)
endforeach()
add_library(py-simulotter MODULE ${python_src})

target_link_libraries(py-simulotter
  simulotter ${SIMULOTTER_LIBS}
  ${SIMULOTTER_PYTHON_LIBS}
  )


foreach(m ${SIMULOTTER_ENABLED_MODULES})
  set(simulotter_modules_apply "${simulotter_modules_apply}SIMULOTTER_MODULES_APPLY_EXPR(${m})\;")
endforeach()
set_property(SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS
  "SIMULOTTER_MODULES_APPLY=${simulotter_modules_apply}")


set_target_properties(py-simulotter PROPERTIES
  LINK_FLAGS "${EXTRA_LINK_FLAGS}"
  OUTPUT_NAME _simulotter PREFIX ""
  )
if(WIN32)
  set_target_properties(py-simulotter PROPERTIES SUFFIX .pyd)
endif()


##
##  Install process
##

if(NOT PYTHON_INSTALL_DIR)
  message(STATUS "PYTHON_INSTALL_DIR not set, use default")
  find_package(PythonInterp)
  if(NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "Python interpreter not found")
  endif()
  # use distutils to find default value for PYTHON_INSTALL_DIR
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
      "from distutils import sysconfig; print sysconfig.get_python_lib(1,0)"
    OUTPUT_VARIABLE PYTHON_INSTALL_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE 
    )
endif()
if(PYTHON_INSTALL_DIR)
  message(STATUS "Install directory for Python module: ${PYTHON_INSTALL_DIR}")
else()
  message(FATAL_ERROR "Python install directory not found.")
endif()


foreach(m __init__ eurobot ${SIMULOTTER_ENABLED_MODULES})
  list(APPEND python_module_files simulotter/${m}.py)
endforeach()
install(FILES ${python_module_files}
  DESTINATION ${PYTHON_INSTALL_DIR}/simulotter/
  COMPONENT python/simulotter/
  )
install(TARGETS py-simulotter
  DESTINATION ${PYTHON_INSTALL_DIR}/simulotter/
  COMPONENT python
  )

# CMake stripping is broken when using absolute destination dir.
install(CODE "
  if(CMAKE_INSTALL_DO_STRIP)
    EXECUTE_PROCESS(COMMAND \"${CMAKE_STRIP}\" \"${PYTHON_INSTALL_DIR}/simulotter/_simulotter.pyd\")
  endif()
  ")

