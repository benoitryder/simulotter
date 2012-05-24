
Eurobot rules
=============

Modules are available for the following Eurobot rules:

.. toctree::

  eurobot2009
  eurobot2010
  eurobot2011
  eurobot2012


.. module:: eurobot

The :mod:`eurobot` module provides default definitions for Eurobot rules. They
are redefined by the `eurobot20XX` modules if needed.


.. data:: WALL_WIDTH
          WALL_HEIGHT

  Wall width and height.

.. data:: RAL

  A color mapping, indexed by RAL numbers.

.. class:: Match(ph=None)

  A class for handling matches, subclassed by `eurobot20XX` modules.
  If *ph* is `None`, a default one is created.

  .. attribute:: physics

    :class:`Physics` instance used to for the match simulation.

  .. attribute:: conf

    Field configuration, rule-specific.

  .. method:: prepare(fconf=None)

    Prepare the game table and game elements using the given field
    configuration as a tuple of values.
    If *fconf* is `None`, a random configuration is used.

The following additional elements are defined by `eurobot20XX` modules:

.. data:: eurobot20XX.TABLE_SIZE

  Table size, as a :class:`vec2`.

.. data:: eurobot20XX.TEAM_COLORS

  Pair of team colors.

.. data:: eurobot20XX.OGround

  The rule-specific ground class.


.. note::
  The modules of past Eurobot rules are usually not maintained once the
  competition is over. The module may not adapted to new features and
  SimulOtter changes

