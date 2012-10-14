cmake_minimum_required(VERSION 2.6)

project(SimulOtter)


##
##  General declarations
##


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif()


##
##  Modules
##

# Define a simulotter module
macro(simulotter_declare_module _m)
  string(TOUPPER ENABLE_${_m} _opt_name)
  option(${_opt_name} "Enable module ${_m}" TRUE)
  if(${_opt_name})
    list(APPEND SIMULOTTER_ENABLED_MODULES ${_m})
  endif()
endmacro()

# Declare known modules
simulotter_declare_module(eurobot2009)
simulotter_declare_module(eurobot2010)
simulotter_declare_module(eurobot2011)
simulotter_declare_module(eurobot2012)


##
##  Dependencies
##

# Macro to (temporarily) prefer static libs over shared.
# Should be followed by a call to prefer_static_restore.
macro(prefer_static_set)
  set(_prefer_static_orig_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
  if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib "${CMAKE_FIND_LIBRARY_SUFFIXES}")
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a "${CMAKE_FIND_LIBRARY_SUFFIXES}")
  endif()
endmacro()

macro(prefer_static_restore)
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${_prefer_static_orig_SUFFIXES}")
endmacro()


##  Bullet
# The built-in Bullet package does not work well.

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


## Freeglut / GLUT

if(USE_FREEGLUT OR Freeglut_USE_STATIC_LIBS)
  find_path(GLUT_INCLUDE_DIR
    NAMES GL/freeglut.h
  )
  if(NOT GLUT_INCLUDE_DIR)
    message(FATAL_ERROR "Freeglut include path not found.")
  endif()

  set(Freeglut_LIB_NAMES libfreeglut freeglut glut)
  if(Freeglut_USE_STATIC_LIBS OR GLUT_USE_STATIC_LIBS)
    prefer_static_set()
    if(WIN32)
      set(Freeglut_LIB_NAMES libfreeglut_static freeglut_static ${Freeglut_LIB_NAMES})
    endif()
  endif()

  find_library(GLUT_LIBRARY NAMES ${Freeglut_LIB_NAMES})
  if(NOT GLUT_LIBRARY)
    message(FATAL_ERROR "Freeglut library not found.")
  endif()
  set(GLUT_LIBRARIES "${GLUT_LIBRARY}")

  if(Freeglut_USE_STATIC_LIBS OR GLUT_USE_STATIC_LIBS)
    prefer_static_restore()
  endif()
  set(GLUT_DEFINITIONS "")
  if(WIN32)
    if(Freeglut_USE_STATIC_LIBS OR GLUT_USE_STATIC_LIBS)
      set(GLUT_DEFINITIONS -DFREEGLUT_STATIC)
    endif()
    list(APPEND GLUT_LIBRARIES winmm)
  endif()
else()
  if(GLUT_USE_STATIC_LIBS)
    prefer_static_set()
  endif()
  find_package(GLUT)
  if(GLUT_USE_STATIC_LIBS)
    prefer_static_restore()
  endif()
endif()


# Other dependencies
if(SDL_USE_STATIC_LIBS)
  prefer_static_set()
endif()
set(SDL_BUILDING_LIBRARY 1)
find_package(SDL)
if(SDL_USE_STATIC_LIBS)
  prefer_static_restore()
endif()
find_package(OpenGL)

if(PNG_USE_STATIC_LIBS)
  prefer_static_set()
endif()
find_package(PNG)
if(PNG_USE_STATIC_LIBS)
  prefer_static_restore()
endif()


#XXX fix a segfault bug on some systems
if(UNIX AND CMAKE_COMPILER_IS_GNUCXX)
  list(INSERT OPENGL_LIBRARIES 0 stdc++) 
endif()

set(SIMULOTTER_LIBS ${BULLET_LIBRARIES}
  ${SDL_LIBRARY} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES}
  ${PNG_LIBRARIES})

include_directories(
  ${CMAKE_SOURCE_DIR} ${BULLET_INCLUDE_DIR}
  ${SDL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
  ${PNG_INCLUDE_DIR}
  )

add_definitions(
  ${GLUT_DEFINITIONS}
  ${PNG_DEFINITIONS}
  )


##
##  Project targets
##

set(simulotter_lib_src
  physics.cpp display.cpp object.cpp sensors.cpp robot.cpp galipeur.cpp graphics.cpp log.cpp colors.cpp
  )
foreach(m ${SIMULOTTER_ENABLED_MODULES})
  list(APPEND simulotter_lib_src modules/${m}.cpp)
endforeach()
add_library(simulotter STATIC ${simulotter_lib_src})

if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
  set(EXTRA_LINK_FLAGS "-Wl,--enable-auto-import -mconsole -mwindows -static-libgcc -static-libstdc++")
else()
  set(EXTRA_LINK_FLAGS)
  if(CMAKE_COMPILER_IS_GNUCXX)
    SET_TARGET_PROPERTIES(simulotter PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()


add_subdirectory(python)

