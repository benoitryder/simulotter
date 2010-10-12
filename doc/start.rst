
Starting with Simulotter
========================

SimulOtter is a robot simulator designed for `Eurobot
<http://www.eurobot.org>`__ rules. It uses OpenGL/SDL for display and `Bullet
<http://www.bulletphysics.com>`__ as physics engine.

SimulOtter is not a stand-alone application but available as a Python module.
Core features are intended to be used in scripts (which may be very simple!)
for testing strategies or just visualize the game table.


Building SimulOtter
-------------------

SimulOtter is known to successfully build on Linux using GCC and on Windows
using MinGW/MSYS. Any feedback on other platforms is welcome!

SimulOtter uses `CMake <http://cmake.org>`_ as build system.
Get it `from here <http://cmake.org/cmake/resources/software.html>` or from
your the packages of your distribution (Linux users).

.. note::
  Do not use the Makefile available in SimulOtter sources. It is here for
  developer needs and is not supported.

The following dependencies are needed:

- Bullet;
- OpenGL, SDL and Freeglut;
- Boost with the Boost.Python component;
- Python 2.6 or 2.7.


Building Bullet
~~~~~~~~~~~~~~~

Bullet packages or binaries are not available.
It is advised to compile it yourself.

First, `get the sources <http://code.google.com/p/bullet/downloads/list>`__
(preferably the last version). It is recommended to build Bullet using CMake.
To install libraries and avoid compiling extra stuff, run CMake with the
following options::

  cmake -DINSTALL_LIBS=ON -DBUILD_EXTRAS=OFF -DBUILD_DEMOS=OFF -DUSE_GLUT=OFF

On Windows, using MinGW/MSYS, you should provide the MinGW base directory::

  cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=/mingw -DINSTALL_LIBS=ON -DBUILD_EXTRAS=OFF -DBUILD_DEMOS=OFF -DUSE_GLUT=OFF

Then, run the usual ``make && make install``.


Other dependencies
~~~~~~~~~~~~~~~~~~

All Linux distributions should provide packages for the other dependencies.
On Debian-based Linux distributions (including Ubuntu), install the following packages:

  libsdl-dev python-dev libboost-python-dev freeglut3-dev

On Windows, sources and/or binaries can be retrieved from the official websites.

- SDL: http://www.libsdl.org/download-1.2.php
- Freeglut: http://freeglut.sourceforge.net/index.php#download
- Boost: http://www.boost.org/users/download/
- Python: http://www.python.org/download/

On Windows, if Python is installed on your system, you should already have the needed files.

Since Boost.Python is not a header-only library, you will have to compile it
(or get precompiled binaries). Windows users should refer to `this
documentation <http://www.boost.org/doc/libs/release/more/getting_started/windows.html>`__.
When invoking ``bootstra.sh``, provide the ``--with-python``
option with your Python installation directory. When invoking ``bjam``, add
``python`` to the arguments to build Boost.Python component.


Building the Python module
~~~~~~~~~~~~~~~~~~~~~~~~~~

Once all dependencies have been installed, run CMake in the SimulOtter source directory.
It is recommended to build in a separate directory and to set the directory in
which the Python module will be installed, using the *PYTHON_INSTALL_DIR*
option (defaults use Python's standard installation location). For instance, to
install it in a ``install/`` subdirectory in SimulOtter sources::

  mkdir build && cd build
  cmake .. -DPYTHON_INSTALL_DIR=$PWD/../install

Some options are supported to prefer the use of static libraries over dynamic
ones; this may be useful, especially on Windows. These options are not
documented (yet?); please, refer to ``CMakeLists.txt`` for details.

After running CMake, do the usual ``make && make install``.
You should have a ``simulotter`` directory containing the Python module
installed in your *PYTHON_INSTALL_DIR*.
Based on the previous example, you should be able to do::

  cd ../install
  python -c 'import simulotter'

