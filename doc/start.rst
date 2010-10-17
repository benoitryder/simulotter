
Starting with SimulOtter
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


A sample Python script
----------------------

In order to set up a simulation you will need to create a Python script.
Basically, it will do the following:

1. create and configure :class:`Physics` and :class:`Display` objects;
#. add game objects to the world (usually, using an `eurobot20XX` module);
#. create robots and prepare their strategy;
#. run the simulation.


The basics
~~~~~~~~~~

First of all, we need to import the `simulotter` module. we will use
:mod:`eurobot2011` rules, so we import this module too. ::

  import simulotter as so
  import simulotter.eurobot2011 as eb
  import math  # always useful to have it

Then, we create the physical world and the display.
We will also bind the *PrintScreen* key to save a screenshot. ::

  ph = so.Physics()
  di = so.Display()
  di.physics = ph
  
  di.set_handler(lambda d,ev: d.screenshot("simu.png"), di.KEYUP, key='PRINT')

We have our physical world. Let's add game table and elements into it, using
the :class:`Match` class provided by the imported `eurobot20XX`. ::

  match = eb.Match(ph)
  match.prepare()


Last but not least, our robot. We make it inherit from the basic
:class:`Galipeur` and place it in a starting area. ::

  class GTest(so.Galipeur):

    def __init__(self, mass, match, team=0):
      so.Galipeur.__init__(self, mass)
      self.match = match

      # asserv configuration
      self.set_speed_xy(1, 2)
      self.set_speed_a(math.pi, math.pi/4)
      self.set_speed_steering(0.5, 1)
      self.set_speed_stop(0.1, 0.5)
      self.set_threshold_stop(0.01, math.pi*5/180)
      self.set_threshold_steering(0.05)

      # position
      k = -1 if team == 0 else 1
      self.pos = so.vec3(
          k*(eb.TABLE_SIZE.x-match.ground.start_size)/2,
          (eb.TABLE_SIZE.y-match.ground.start_size)/2,
          0.1)

      # don't forget to add it to the world
      self.addToWorld(match.physics)

  g = GTest(4, match)

Finally, we start the simulation. ::

  di.run()
  di.close()  # close the display after aborting run()


There we are. Running the script will display the game table with our robot on
its starting area.


Set up a strategy
~~~~~~~~~~~~~~~~~

A static robot is boring. It's time to make it move!
We will define a strategy method on our robot that will be scheduled to start
at the beginning of the match. ::

  def strategy(self):
    # prepare to push the center pawn
    self.order_xya( eb.SQUARE_SIZE*so.vec2(-2.5,2.5), math.pi/3 )
    while not self.is_waiting(): yield  # wait order to be executed
    # push the center pawn
    self.order_xy( eb.SQUARE_SIZE*so.vec2(1.0,-1.0) )
    while not self.is_waiting(): yield
    # move above the pawn...
    self.order_xya( eb.SQUARE_SIZE*so.vec2(1.5,0.5), 0 )
    while not self.is_waiting(): yield
    # ... and push it in the the safe zone (relative move)
    self.order_xy( so.vec2(y=-2.5*eb.SQUARE_SIZE+self.D_SIDE), True )
    while not self.is_waiting(): yield

    # end of the strategoy: stop the robot, shut off the asserv
    self.order_stop()
    self.task_asserv.cancel()

In the constructor, we schedule the asserv and the strategy.
The strategy will start 1 second after the simulation start. ::

  def __init__(self, mass, match, team=0):
    ...
    # schedule the asserv and the strategy
    self.task_asserv = match.physics.schedule(lambda ph: self.asserv(), period=0.1)
    self.task_strat = match.physics.schedule(self.strategy(), period=0.2, time=1)

.. note::
  The strategy method is a generator, that is why we schedule
  ``self.strategy()`` (an iterator) and not ``self.strategy``.

