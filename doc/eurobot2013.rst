
.. _eurobot2013:

Eurobot 2013 --- *Happy Birthday!*
----------------------------------

.. module:: eurobot2013

.. class:: OGround()

  Ground for Eurobot 2013.

.. data:: OGround.SIZE

  Size of the ground as a :class:`vec2`.

.. data:: OGround.SQUARE_SIZE

  Size of starting area.


.. class:: OCake()

  The birthday cake. Inherit from :class:`OSimple`.

.. data:: OCake.LEVEL_HEIGHT
          OCake.LEVEL_RADIUS
          OCake.BASE_RADIUS
          OCake.BASKET_HEIGHT
          OCake.BASKET_RADIUS
          OCake.BASKET_WIDTH

  Various cake measurements.


.. class:: OGiftSupport()

  Gift support and its gifts. Inherit from :class:`OSimple`.

  Origin is located at the anchor point on the table.

.. data:: OGiftSupport.SIZE

  Gift support size, as a :class:`vec3`.


.. class:: OGlass()

  Glass. Inherit from :class:`OSimple`.

.. data:: OGlass.SIZE
          OGlass.HEIGHT
          OGlass.RADIUS
          OGlass.INNER_RADIUS
          OGlass.BOTTOM_HEIGHT

  Various glass measurements.


.. class:: OCandle()

  Candle and its flame. Inherit from :class:`OSimple`.

.. data:: OGlass.HEIGHT
          OGlass.RADIUS

  Various candle measurements.


.. class:: OPlate()

  A cherry plate. Inherit from :class:`OSimple`.

.. class:: OCherry()

  A cherry. Inherit from :class:`OSimple`.


.. class:: Match()

  .. attribute:: ground

    The :class:`OGround` instance.


