
.. _eurobot2011:

Eurobot 2011 --- *Chess up!*
----------------------------

.. module:: eurobot2011

.. data:: SQUARE_SIZE

  Size of chessboard squares.

.. data:: RANDOM_POS

  Random positions of king and queen, as pairs. 0 is top, 4 is bottom.

.. class:: OGround()

  Ground for Eurobot 2011.

.. class:: OPawn()

  A pawn. Inherit from :class:`OSimple`.
  Pawns are magnetized on their two faces.

.. class:: OKing()

  A king. Inherit from :class:`OPawn`.

.. class:: OQueen()

  A queen. Inherit from :class:`OPawn`.

.. data:: OPawn.RADIUS
          OPawn.HEIGHT

  Radius and height of a simple pawn (without figure).


.. class:: Galipeur(mass)
  
  Galipeur adapted to Eurobot 2011 rules, with magnetic arms to grab pawns.

  .. method:: set_arm_av(av)

    Set angle velocity of arms. If value is to small arms will not be able to
    raise while grabbing a pawn. A value of ``2*math.pi`` should be fine.

  .. attribute:: arms

    Pair of arms, as a tuple of `Galipeur.PawnArm` instances.

  .. class:: PawnArm

    This class cannot be instantiated from Python.

    A Galipeur's magnetic arm, to move pawns. The arm is
    initially raised with magnet enabled.

    .. method:: angle

      Arm angle, between 0 (down) and π/2 (up).

    .. method:: up()

      Raise the arm.

    .. method:: down()

      Lower the arm.

    .. method:: grab()

      Enable  the magnet, allowing pawn grabbing.

    .. method:: release()

      Disable the magnet, releasing grabbed pawns.

.. data:: Galipeur.ARM_RADIUS
          Galipeur.ARM_LENGTH

  Radius and length of Galipeur arms.
  Length does not include size of caps at both ends.


.. class:: Match()

  Field configuration is a ``(king_and_queen, line1, line2)`` 3-uple of random
  card numbers, from 0 to 19 (inclusive).

  .. attribute:: pawns

    List of :class:`OPawn` objects.

  .. attribute:: kings
                 queens

    Pairs of :class:`OKing` and :class:`OQueen` objects.

  .. attribute:: ground

    The :class:`OGround` instance.


