
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

.. class:: OKing()

  A king. Inherit from :class:`OPawn`.

.. class:: OQueen()

  A queen. Inherit from :class:`OPawn`.


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


