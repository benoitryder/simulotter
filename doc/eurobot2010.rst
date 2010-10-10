
.. _eurobot2010:

Eurobot 2010 --- *Feed the World*
---------------------------------

.. module:: eurobot2010

.. data:: FIELD_POS_DXY

  Space between field elements, as a :class:`vec2`.

.. data:: FIELD_POS_Y0

  Distance from the table's bottom to the lowest field element.

.. function:: field_pos(x, y)

  Convert a field position to world coordinates. Return a :class:`vec2`.
  Element position ``(0, 0)`` is the middle bottom.

.. data:: TOMATOES_FPOS
          CORNS_FPOS

  Field positions of tomatoes and ears of corn.

.. data:: FAKES_FPOS_SIDE
          FAKES_FPOS_CENTER

  Field positions of fake ears of corns, indexed by configuration values.


.. class:: ORaisedZone()

  Raised zone, with orange trees on it. Inherit from :class:`OSimple`.

.. data:: WIDTH
          HEIGHT
          BOTTOM_LENGTH
          TOP_LENGTH
          STRIP_LENGTH
          WALL_WIDTH
          WALL_HEIGHT
          WALL_BOTTOM_LENGTH
          WALL_TOP_LENGTH

  Various dimensions of the raised zone.


.. class:: OCorn()

  An ear of corn. Inherit from :class:`OSimple`.

  .. method:: plant(x, y)

    Plant the ear of corn at given coordinates.

  .. method:: uproot()

    Uproot the ear of corn. It will then fall to the ground.

.. class:: OCornFake()

  A fake (black) ear of corn. Inherit from :class:`OSimple`.

.. class:: OTomato()

  A tomato. Inherit from :class:`OSimple`.

.. class:: OOrange()

  An orange. Inherit from :class:`OSimple`.

.. class:: OBranch(h)

  A branch an orange tree. Inherit from :class:`OSimple`.
  *h* is the height of the branch.

  .. method:: createOrange()

    Create an orange an put in on the branch.

  .. classmethod:: branchPos(x, y, h)

    Compute position of a given branch for a given tree.
    *x* and *y* are -1 or 1 and give the tree position.
    *h* is the branch height, provided to the constructor.

.. class:: OBac(team)

  A collecting bac.

  .. method:: contains(obj)

    Return `True` if the *obj* object is in the bac.
    Test is based on object's center of mass position.

.. data:: OBac.XY0
          OBac.SIZE
          OBac.WIDTH
          OBac.HEIGHT

  Various dimensions of bacs.


.. class:: Match()

  Field configuration is a ``(side, center)`` 2-uple
  where ``0 <= side <= 8`` and ``0 <= center <= 3``.

  .. attribute:: tomatoes
                 corns
                 oranges

    Lists of tomatoes, ears of corn and oranges created for the match.

  .. attributes

    A pair of created bacs, one per team.

  .. attribute:: ground

    The :class:`OGround` instance.

  .. method:: scores()

    Return current scores as a pair, one element per team.

  .. method:: isCollected(obj)

    Return a team number if the given object is in a bac, `False` otherwise.


