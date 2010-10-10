
.. _robots:

Robots
======

Robots inherit from the :class:`Robot` abstract class which itself inherit from
:class:`Object`.

Currently, two robot sub-classes are defined: :class:`RBasic` for simple robot
with basic orders and :class:`Galipeur`, the holonomic robot of the Rob'Otter
team.


.. class:: Robot

  Base class for robots, inherit from :class:`Object`.


Basic robot --- :class:`RBasic`
-------------------------------

A basic robot with XY and angle orders. It uses a very simple asserv algorithm
to reach its target.

.. class:: RBasic(shape, mass)

  Return a new basic robot with given shape and mass.

  .. attribute:: color

    Object's color. Defaults to black.

  .. attribute:: a

    Rotation angle around Z axis.

  .. attribute:: v

    XY linear velocity as a :class:`vec2`.

  .. attribute:: av

    Angular velocity around Z axis.

  .. attribute:: v_max

    Maximum XY linear velocity as a float (norm of :attr:`v`).

  .. attribute:: av_max

    Maximum angular velocity around Z axis.

  .. attribute:: threshold_xy

    XY distance tolerancy to reach target position.

  .. attribute:: threshold_a

    Angle tolerancy to reach target angle.

  .. method:: asserv()

   Basic asserv step. First, the robot turns to face the target, then
   move forward to it.

   This method must be called at regular interval for the robot to execute
   orders.

  .. method:: order_xy(xy, rel=False)

    Set target position to *xy*. If *rel* is `True`, *xy* is relative to
    the current robot position.

  .. method:: order_a(xy, a, rel=False)

    Set target angle to *a*. If *rel* is `True`, *a* is relative to the current
    robot angle.

  .. method:: order_xya(xy, a, rel=False)

    Equivalent to ``order_xy(xy, rel); order_a(a, rel)``.

  .. method:: order_back(d)

    Order the robot to move back of the given distance.

  .. method:: order_stop()

    Order the robot to stop. Cancel current orders.

  .. method:: is_waiting()

    Return `False` if the robot is currently executing orders, `True` otherwise
    (target reached).


Galipeur --- :class:`Galipeur`
------------------------------

.. currentmodule:: Galipeur

Galipeur's asserv uses checkpoint-base trajectories for position and an
additional final angle target.

The `set_speed_*()` methods configure velocities and accelerations between
checkpoints (`set_speed_xy()`), over an intermediate checkpoint
(`set_speed_steering()`) and to reach the mast checkpoint (`set_speed_stop()`).

.. warning::
  These settings default to 0. Thus, Galipeur will not move unless they are
  explicitely set to appropriated values.
  

The `set_threshold_*()` methods configure distance and angle tolerancies to
consider that a target checkpoint or angle has been reached.

.. currentmodule:: None

.. class:: Galipeur(mass)

  Return a new Galipeur with given mass.

  .. attribute:: color

    Galipeur's color. Defaults to gray 30%.

  .. attribute:: a

    Rotation angle around Z axis.

  .. attribute:: v

    XY linear velocity as a :class:`vec2`.

  .. attribute:: av

    Angular velocity around Z axis.

  .. method:: asserv()

   Execute an sserv step. This method must be called at regular interval for
   Galipeur to execute orders.

  .. method:: order_xy(xy, rel=False)

    Set a new trajectory with a unique point given by *xy*.
    If *rel* is `True`, *xy* is relative to
    the current robot position.

  .. method:: order_a(xy, a, rel=False)

    Set target angle to *a*. If *rel* is `True`, *a* is relative to the current
    robot angle.

  .. method:: order_xya(xy, a, rel=False)

    Equivalent to ``order_xy(xy, rel); order_a(a, rel)``.

  .. method:: order_stop()

    Order to stop. Cancel current orders.

  .. method:: order_trajectory(iterable)

    Set a new trajectory from an iterable of :class:`vec2`.

  .. method:: order_xy_done()

    Return `True` if the robot is stopped or on the target position, `False`
    otherwise.

  .. method:: order_a_done()

    Return `True` if the robot is stopped or has the targetted angle, `False`
    otherwise.

  .. method:: is_waiting()

    Equivalent to ``order_xy_done() and order_a_done()``.

  .. method:: current_checkpoint()

    Return the current zero-based checkpoint index.

  .. method:: test_sensor(i)

    Hit-test for the sensor with index *i*. Return the collision distance or
    `None` is the sensor did not hit.

  .. method:: set_speed_xy(v, acc)

    Set maximum XY cruising velocity and acceleration.

  .. method:: set_speed_a(v, acc)

    Set maximum angle velocity and acceleration.

  .. method:: set_speed_steering(v, dec)

    Set maximum XY velocity and deceleration, when passing over a checkpoint.

  .. method:: set_speed_stop(v, dec)

    Set maximum XY velocity and deceleration, when reaching the last checkpoint.

  .. attribute:: set_threshold_stop(d, a)

    XY distance and angle tolerancy to reach target.

  .. attribute:: set_threshold_steering(d)

    XY distance tolerancy for intermediate checkpoints.


The following class constants are defined:

.. data:: Galipeur.Z_MASS

  Z position of the center of mass.

.. data:: Galipeur.GROUND_CLEARANCE

  Distance between the ground and the bottom of Galipeur's body.

.. data:: Galipeur.ANGLE_OFFSET

  Angle of the first wheel.

.. data:: Galipeur.HEIGHT

  Body height.

.. data:: Galipeur.SIDE

  Triangle side half size.

.. data:: Galipeur.W_BLOCK

  Motor block half width (small side).

.. data:: Galipeur.R_WHEEL

  Wheel radius.

.. data:: Galipeur.H_WHEEL

  Wheel height (when laid flat).

.. data:: Galipeur.D_SIDE

  distance center/triangle side

.. data:: Galipeur.D_WHEEL

  Distance center/wheel side.

.. data:: Galipeur.A_SIDE

  Center angle of triangle side.

.. data:: Galipeur.A_WHEEL

  Center angle of wheel side.

.. data:: Galipeur.RADIUS

  Outer circle radius.

