
Miscellaneous objects 
=====================

Colors --- :class:`Color`
-------------------------

:class:`Color` instances are used to represent RGBA colors. They are
immutable and iterable.

Each color component is stored as a float value, clamped to the range [0,1]. For the alpha component, 1 is fully opaque and 0 is fully transparent.

.. class:: Color(r=0.0, g=0.0, b=0.0, a=1.0)
           Color(r=0, g=0, b=0, a=255)
           Color(gray)

  Return a new color built from its component or a gray level.
  Color components and *gray* are `float` values clamped to the range [0,1].
  The default constructor return a fully opaque black.

  For the component form, if all provided values are `int` they are mapped from
  the range [0,255] to the range [0,1].

  .. attribute:: r
                 g
                 b
                 a

    RGBA components as floats in the range [0,1].
    For *a*, 1 is fully opaque and 0 is fully transparent.

  .. method:: self * float
              float * self

    Scale each component by a given value.

  .. method:: __iter__()

    Return an iterator on color components, in RGBA order.
 
.. data:: Color.white
          Color.black
          Color.plexi

  Class constants for white, black and plexiglass colors.


