
Physical simulation
===================

.. todo:: General description and explanation on physical worlds, objects, etc.

Physical world --- :class:`Physics`
-----------------------------------

A physical world is like a container for all simulated objects. It handles
collisions, dynamics, etc. Several worlds can be created,
simulated and displayed independently.

.. class:: Physics(step_dt=0.002)

  Return a new physical world.

  .. attribute:: step_dt

    Duration of a single simulation step. Lower values mean more accurate
    simulation but require more resources for a realtime simulation.

  .. attribute:: time

    Current simulation time.

  .. method:: step()

    Advance the simulation of one step. :attr:`time` will be increased by the
    value of :attr:`step_dt`.


Class attributes affect elements related to physical worlds, including
configuration of created worlds. These values should be modified at startup if
needed.

.. data:: Physics.world_gravity

  World gravity for new :class:`Physics` instances.

  Defaults to 9.80665.

.. data:: Physics.world_aabb_min
          Physics.world_aabb_max

  Bounding box extremities of new created worlds, as :class:`vec3`.

  Default to ``vec3(-10,-5,-2)`` and ``vec3(10,5,2)``.

.. data:: Physics.world_objects_max

  Maximum number objects that new :class:`Physics` instances can contain.

  Defaults to 300.

.. data:: Physics.margin_epsilon

  Small gap distance used when positioning objects near others to avoid them overlapping.
  Used for instance when setting :attr:`OSimple.pos` to a :class:`vec2` to put an object above the ground.

  Defaults to 0.001.


Simulated objects
-----------------

Objects are physical bodies, either static (e.g. walls) or dynamic (e.g.
:ref:`robots <robots>`, game elements).
They are intended to be put in a :class:`Physics` world. An object can be in at
most one world at a time.

All objects derive from the abstract class :class:`Object`.

.. class:: Object

  The :class:`Object` class cannot be instantiated or derived from Python.

  An object defines the following attributes.

  .. method:: addToWorld(physics)

    Add the object to a :class:`Physics` world.

  .. method:: removeFromWorld()

    Remove the object from its :class:`Physics` world.

  .. attribute:: pos

    Object position vector, as a :class:`vec3`.
    Equivalent to ``object.trans.origin``.

  .. attribute:: rot

    Object rotation matrix, as a :class:`matrix3`.
    Equivalent to ``object.trans.basis``.

  .. attribute:: trans

    Object transformation, as a :class:`trans`.

  .. attribute:: physics

    :class:`Physics` world the object is currently in, or `None`.


Simple object --- :class:`OSimple`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :class:`OSimple` class provide a handy object implementation. It allows to
create static and dynamic bodies with basic geometrical shapes.

.. class:: OSimple([shape, mass=0])

  An object must be initialized before being added to a world. This means its
  shape must be set.

  If object's mass is null (which is the default), the object is static: it
  cannot be moved and does not collide with other static objects.

  .. attribute:: shape

    The object's shape; must be set before adding the object to a world.
    Once set, it cannot be changed.
    cannot be called twice.

  .. attribute:: mass

    Object's mass, or 0 for static objects (the default).
    The object must be initialized before setting its mass.

  .. attribute:: initialized

    Same value as ``object.shape is None``.

  .. attribute:: color

    Object's color. Defaults to black.

  .. attribute:: pos

    Extends :attr:`Object.pos`. If set to a :class:`vec2`, place the object
    above the ground (based on its bounding box) instead of setting *z* to 0.


.. _oground:

Table ground --- :class:`OGround`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. class:: OGround(ground_color, team1_color, team2_color)

  Ground of the game table, with square starting areas in corners, symmetric
  with respect to the y-axis.
  *ground_color* is the main color, *team1_color* and *team2_color* are colors
  of the starting areas.

  When added to a world, a ground is placed so that the center of the top of
  the table is at the world's origin.

  .. attribute:: start_size

    Side size of starting areas. It only affects display and should be set
    before the object is drawn.

    Defaults to 0.5.

.. data:: OGround.SIZE

  Size of grounds, as a :class:`vec2`.

