cmake_minimum_required(VERSION 2.6)

project(SimulOtter)


##
##  General declaration
##


list(APPEND CMAKE_CXX_FLAGS "-Wall -Werror")

if(MSYS)
  # Add /mingw as system prefix.
  execute_process(COMMAND sh -c "cd /mingw && pwd -W"
    OUTPUT_VARIABLE MINGW_ROOT_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  list(APPEND CMAKE_SYSTEM_PREFIX_PATH ${MINGW_ROOT_PATH})
endif()



##
##  Dependencies
##


##  Bullet
# The built-in Bullet package does not work well.
#TODO properly check for errors (not found, ...)

find_path(BULLET_INCLUDE_DIR
  NAMES btBulletCollisionCommon.h
  PATH_SUFFIXES "bullet"
)
if(NOT BULLET_INCLUDE_DIR)
  message(FATAL_ERROR "Bullet include path not found.")
endif()

macro(FIND_BULLET_LIBRARY _v)
  find_library(${_v} NAMES ${ARGN})
  mark_as_advanced(${_v})
  if(NOT ${_v})
    message(FATAL_ERROR "Bullet library not found: ${ARGN}")
  endif()
endmacro()

FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIB  BulletDynamics)
FIND_BULLET_LIBRARY(BULLET_COLLISION_LIB BulletCollision)
FIND_BULLET_LIBRARY(BULLET_MATH_LIB      LinearMath)
set(BULLET_LIBRARIES ${BULLET_DYNAMICS_LIB} ${BULLET_COLLISION_LIB} ${BULLET_MATH_LIB})


## Freeglut

find_path(FREEGLUT_INCLUDE_DIR
  NAMES GL/freeglut.h
)
if(NOT FREEGLUT_INCLUDE_DIR)
  message(FATAL_ERROR "Freeglut include path not found.")
endif()

set(Freeglut_LIB_PREFIX "")
if(Freeglut_USE_STATIC_LIBS)
  set(_Freeglut_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  if(WIN32)
    set(Freeglut_LIB_PREFIX "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif()
else()
endif()

find_library(FREEGLUT_LIBRARY
  NAMES ${Freeglut_LIB_PREFIX}freeglut ${Freeglut_LIB_PREFIX}glut)
if(NOT FREEGLUT_LIBRARY)
  message(FATAL_ERROR "Freeglut library not found.")
endif()

if(Freeglut_USE_STATIC_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_Freeglut_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
endif()
if(WIN32) #AND Freeglut_USE_STATIC_LIBS)
  # always set it, seems to be needed when linking against the DLL
  set(FREEGLUT_DEFINITIONS -DFREEGLUT_STATIC)
else()
  set(FREEGLUT_DEFINITIONS)
endif()


# Other dependencies
set(SDL_BUILDING_LIBRARY 1)
find_package(SDL)
find_package(OpenGL)
find_package(PNG)


set(SIMULOTTER_LIBS ${BULLET_LIBRARIES}
  ${SDL_LIBRARY} ${OPENGL_LIBRARIES} ${FREEGLUT_LIBRARY}
  ${PNG_LIBRARIES})

include_directories(
  ${CMAKE_SOURCE_DIR} ${BULLET_INCLUDE_DIR}
  ${SDL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${FREEGLUT_INCLUDE_DIR}
  ${PNG_INCLUDE_DIR}
  )

add_definitions(
  ${FREEGLUT_DEFINITIONS}
  ${PNG_DEFINITIONS}
  )


##
##  Project targets
##

add_library(simulotter STATIC
  physics.cpp display.cpp object.cpp sensors.cpp robot.cpp galipeur.cpp log.cpp
  modules/eurobot2009.cpp modules/eurobot2010.cpp modules/eurobot2011.cpp
  )

if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
  set(EXTRA_LINK_FLAGS "-Wl,--enable-auto-import -mconsole -mwindows -static-libgcc -static-libstdc++")
else()
  set(EXTRA_LINK_FLAGS)
endif()


add_subdirectory(python)

