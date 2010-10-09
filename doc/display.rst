
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

  .. method:: set_handler(cb, type, \*\*kw)

    Set or remove an event handler.
    The handler is defined by its *type* and additional type-specific keyword arguments.
    If *cb* is `None` the current handler (if any) is removed. Otherwise, *cb*
    must be a callable defined as ``cb(display, event)``.

    See :ref:`display-event-handlers` for details.

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

  .. attribute:: paused

    If true, the :attr:`physics` world will not be stepped by :meth:`run`.

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


.. _display-event-handlers:

Event handlers
--------------

.. currentmodule:: Display

Event handlers allow to bind actions to input events. They are set using
:meth:`set_handler`. An event handler callback takes two parameters: the
:class:`Display` which has triggered the event and event information as a
:class:`Display.Event` instance.


Event types
~~~~~~~~~~~

Events define a :attr:`type` attribute and type-specific attributes which
provide additional information. To set an event handler, one must provide its
type and some specific fields to define events that will match.

Event types and their specific attributes are described below.
They are defined on :class:`Display` and also on :class:`Display.EventType`.

.. data:: KEYDOWN
          KEYUP

  A key has been pressed (`KEYDOWN`) or released (`KEYUP`).

  .. note::
    Key repeat is enabled. Thus, `KEYUP` is recommanded for single
    actions (e.g. pause, screenshots, ...).

  For :class:`set_handler`, the `key` keyword is required.

  .. attribute:: Event.key

    They pressed/released :ref:`key <display-keys>`.

  .. attribute:: Event.mod

    Current state of :ref:`modified keys <display-keys>`.

.. data:: MOUSEMOTION

  Mouse has been moved.

  For :class:`set_handler`, the `state` keyword is required.

  .. attribute:: Event.state

    :ref:`Mouse button <display-mouse-buttons>` state, as a bit mask.

  .. attribute:: Event.pos

    Mouse coordinates as a ``(x, y)`` tuple.

  .. attribute:: Event.rel

    Relative mouse motion as a ``(x, y)`` tuple.

.. data:: MOUSEBUTTONDOWN
          MOUSEBUTTONUP

  A mouse button has been pressed (`MOUSEBUTTONDOWN`) or released (`MOUSEBUTTONUP`).

  For :class:`set_handler`, the `button` keyword is required.

  .. attribute:: Event.button

    A :ref:`mouse button <display-mouse-buttons>`.

  .. attribute:: Event.pos

    Mouse coordinates as a ``(x, y)`` tuple.


.. _display-keys:

Key values and modifiers
~~~~~~~~~~~~~~~~~~~~~~~~

Key values can be provided to :meth:`set_handler` using attributes defined on
:class:`Key`, their names (as strings) or single ASCII characters (for the
associatede key).

.. warning:: On Windows keys are those of the QWERTY US keyboard.


:class:`Key` attributes names are similar to `values defined by SDL
<http://www.libsdl.org/docs/html/sdlkey.html>`_ with minor adjustements
(``WORLDN`` values removed, ``0`` to ``9`` prefixed by ``DIGIT``). Here is the
complete list (take a breath):

  `BACKSPACE`, `TAB`, `CLEAR`, `RETURN`, `PAUSE`, `ESCAPE`, `SPACE`,
  `EXCLAIM`, `QUOTEDBL`, `HASH`, `DOLLAR`, `AMPERSAND`, `QUOTE`, `LEFTPAREN`,
  `RIGHTPAREN`, `ASTERISK`, `PLUS`, `COMMA`, `MINUS`, `PERIOD`, `SLASH`,
  `DIGIT0` to `DIGIT9`, `COLON`, `SEMICOLON`, `LESS`, `EQUALS`, `GREATER`,
  `QUESTION`, `AT`, `LEFTBRACKET`, `BACKSLASH`, `RIGHTBRACKET`, `CARET`,
  `UNDERSCORE`, `BACKQUOTE`, `a` to `z`,
  `DELETE`, `KP0` to `KP9` `KP_PERIOD`, `KP_DIVIDE`, `KP_MULTIPLY`, `KP_MINUS`,
  `KP_PLUS`, `KP_ENTER`, `KP_EQUALS`, `UP`, `DOWN`, `RIGHT`, `LEFT`, `INSERT`,
  `HOME`, `END`, `PAGEUP`, `PAGEDOWN`, `F1` to `F15` `NUMLOCK`, `CAPSLOCK`,
  `SCROLLOCK`, `RSHIFT`, `LSHIFT`, `RCTRL`, `LCTRL`, `RALT`, `LALT`, `RMETA`,
  `LMETA`, `LSUPER`, `RSUPER`, `MODE`, `COMPOSE`, `HELP`, `PRINT`, `SYSREQ`,
  `BREAK`, `MENU`, `POWER`, `EURO`, `UNDO`

For :attr:`Event.mod` the following modifiers are defined on :class:`KMod`.
They can be OR'd.

  | `NONE` (no modifier)
  | `LSHIFT`, `RSHIFT`
  | `LCTRL`, `RCTRL`
  | `LALT`, `RALT`
  | `LMETA`, `RMETA`
  | `NUM`, `CAPS`, `MODE`
  | `CTRL` (equal to ``LCTRL|RCTRL``)
  | `SHIFT` (equal to ``LSHIFT|RSHIFT``)
  | `ALT` (equal to ``LALT|RALT``)
  | `META` (equal to ``LMETA|RMETA``)


.. _display-mouse-buttons:

Mouse buttons
~~~~~~~~~~~~~

Mouse buttons are represented as integer values, starting from 1.
Usually:

- 1 is left button;
- 2 is middle button;
- 3 is right button;
- 4 is mouse wheel up;
- 5 is mouse wheel down.

For the :const:`MOUSEMOTION` event, :attr:`Event.state` is a bit mask. Bit
indexes are button values given above. For instance, 0 means no button and
``0b101`` means both left and right buttons.


Default handlers
~~~~~~~~~~~~~~~~

The following default handlers are defined. They can be overridden using :meth:`set_handler`.

- *escape*: close the display
- *WASDQE*: move the camera origin along X/Y/Z axes
- *left click + mouse move*: change orientation orientation
- *R*: reset camera to default
- *space*: pause the simulation

.. note::
  Actually, defaults camera moves are mapped to ZQSDAE according to the French
  AZERTY keyboard. This will have no impact on Windows since keys are detected
  according to the QWERTY US keyboard.

Examples
~~~~~~~~

::

  from simulotter import *

  di = Display()

  # save a screenshot with the 'print screen' key
  di.set_handler(lambda d,ev: d.screenshot("simu.png"), di.KEYUP, key='PRINT')

  # speed up/down the simulation with the mouse wheel
  def handler_time_scale(d, ev):
    if ev.button == 4:
      d.time_scale += 0.5
    elif ev.button == 5:
      d.time_scale -= 0.5
  di.set_handler(handler_time_scale, di.MOUSEBUTTONDOWN, button=4)
  di.set_handler(handler_time_scale, di.MOUSEBUTTONDOWN, button=5)

  # disable default 'left click + mouse move' handler
  di.set_handler(None, di.MOUSEMOTION, state=1)

