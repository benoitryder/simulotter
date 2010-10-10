
.. _eurobot2009:

Eurobot 2009 --- *Temples of Atlantis*
--------------------------------------

.. module:: eurobot2009

.. data:: COL_SPACE_XY

  Space between adjacent column elements on the table, as a :class:`vec2`.

.. data:: COL_OFFSET_XY

  Offset for column elements position on the table, as a :class:`vec2`.

.. data:: COL_POS

  Column element positions, as 3-uple, indexed by random configuration.

.. class:: OColElem()

  A column element. Inherit from :class:`OSimple`.

.. class:: OLintel()

  A lintel. Inherit from :class:`OSimple`.

.. class:: ODispenser()

  A column element dispenser. Inherit from :class:`OSimple`.

  .. method:: setPos(attach, side)

    Set position from attach point position and wall side.
    ``attach.z`` is the space between the dispenser and the ground.
    *side* is the wall side (0: top, 1: right, 2: bottom, 3: left).

  .. method:: fill(obj, z)

    Put an object in the dispenser, at height *z*.
    *obj* should be a :class:`OColElem``.

.. data:: ODispenser.RADIUS
          ODispenser.HEIGHT

  Radius and height of dispensers.

.. class:: OLintelStorage()

  A lintel storage. Inherit from :class:`OSimple`.

  .. method:: setPos(x, side)

    Set position from X attach point position (on the wall) and wall side.
    ``attach.z`` is the space between the dispenser and the ground.
    *side* is the wall side (0: top, 1: right, 2: bottom, 3: left).

  .. method:: fill(obj)

    Put an object in the dispenser. *obj* should be a :class:`OLintel``.

.. class:: Galipeur([mass])

  Galipeur adapted to Eurbot 2009 rules, with a so-called *pàchev*.

  .. attribute:: pachev_pos

    The pàchev Z position.

  .. method:: order_pachev_move(z)

    Move the pàchev to the given Z position.

  .. method:: order_pachev_release()

    Open the pàchev, release grabbed objects.

  .. method:: order_pachev_grab

    Close the pàchev, grab elements.

  .. method:: order_pachev_eject

    Open the pàchev and eject elements it contains.

  .. method:: set_pachev_v

    Set pàchev velocity (along Z axis).

  .. method:: set_threshold_pachev

    Set tolerancy for pàchev moves. velocity (along Z axis).

  .. attribute:: pachev_eject_speed

    Speed at which elementw will be ejected from the pàchev.


.. class:: Match()

  Field configuration is a ``(columns, dispensers)`` 2-uple where
  ``0 <= columns <= 9`` and ``dispensers in (0, 1)``.

  .. attribute:: ground

    The :class:`OGround` instance.


