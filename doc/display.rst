
Simulation display
==================

Display screen --- :class:`Display`
-----------------------------------

A display is basically a window displaying the 3D simulated world and handling
user input (keyboard mouse). It is is not needed to run a simulation. 

Display is based on SDL and OpenGL and thus does not support creation of
multiple windows.


.. class:: Display()

  Return a new display.

  .. method:: run()

    Display the simulation and run it by stepping the :attr:`physics` world is
    stepped. Timings are given by :attr:`fps` and :attr:`time_scale`.
    The display is opened if needed.
    Call :meth:`abort` to make the method return.

  .. method:: abort()

    Abort a current call to :meth:`run`.

  .. method:: update()

    Refresh display. Do not step the simulation.
    The display is opened if needed.

  .. method:: resize(width, height, mode=None)

    Open or resize the display.
    *mode* allows to choose between fullscreen mode (`True`) or window mode
    (`False`). If *mode* is `None`, the previous mode is used.

    Default mode is 800x600 window mode.

  .. method:: close()

    Close the display. If it is not opened an error is raised.

  .. method:: screenshot(filename)

    Save a PNG screenshot to a file.

  .. attribute: physics

    :class:`Physics` world to display. Required to start the display.

  .. attribute:: screen_size

    Return current screen size as a ``(width, height)`` pair.

  .. attribute:: time_scale

    Time scale used by :meth:`run`. Values greater than 1, slow down the
    simulation. Values less than 1, speed up the simulation.

    Defaults to 1.
    
  .. attribute:: fps

    Framerate in frames per second used by :meth:`run`.

    Defaults to 60.

  .. attribute:: bg_color

    Background color.

    Defaults to gray 80%.

  .. attribute:: camera

    The display :class:`Camera`.


The following class attributes are cached. Changes will not be effective until
reopening or resizing a display.

.. attribute:: Display.draw_div

  Slices and stacks for rounded geometry objects such as spheres.

  Defaults to 20.

.. attribute:: Display.draw_epsilon

  Gap between contiguous surfaces. It is used to display a surface above
  another without visual glitch (e.g. for :ref:`ground <oground>` starting
  areas).

  Defaults to 0.5mm.

.. attribute:: Display.antialias

  Multisampling count providing an antialising effect, 0 to disable.
  The higher, the better; however, accepted values depends on the video
  rendering device.


Display camera --- :class:`Display.Camera`
------------------------------------------

.. class:: Display.Camera

  This class cannot be instantiated from Python.

  :attr:`trans` gives the camera transformation from the camera referential.
  If :attr:`obj` is not `None` it is ``obj.trans``. Otherwise, the world's
  referential is used.

  The camera is oriented along the negative Z axis (towards the ground) with
  vertical Y axis and horizontal X axis.

  .. attribute:: trans

    Camera position and direction as a :class:`trans`.

  .. attribute:: obj

    :class:`Object` used as referential for camera transformation or `None`.
    Used for embedded camera.

  .. attribute:: fov

    Vertical field of view, in degrees.

    Defaults to 45°.

  .. attribute:: z_near
                 z_far

    Distance of near and far clipping planes.

    Defaults to 0.1m and 300m.


