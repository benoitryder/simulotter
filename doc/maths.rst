
Mathematical objects
====================

Various classes for geometric calculations.

Mathematical objects are immutable: they cannot be updated in place.
Some implicit conversions (for instance, from :class:`vec2` to
:class:`vec3`). This is convenient but may be tricky when using operators:
``vec3(3,3,3) + vec2(2,2)`` returns ``vec3(5,5,3)`` but
``vec2(2,2) + vec3(3,3,3)`` returns ``vec2(5,5)``.

Some mathematical objects are iterable and as such, can be converted to `tuple`
or `list` and expanded to function arguments. This is particularly useful for
vectors: if *v* is a `vec3`, then ``f(*v)`` is equivalent to ``f(v.x, v.y,
v.z)``.


Vectors --- :class:`vec2`, :class:`vec3`
----------------------------------------

.. todo:: What is the difference between q.rotate(v) and q * v ?

.. class:: vec2(x=0, y=0)
           vec3(x=0, y=0, z=0)

  Return a new 2D or 3D vector.


  .. attribute:: x
                 y
                 z

    Vector coordinates. *(z only for vec3)*

  .. method:: dot(vec)

    Return the dot product with another vector.

  .. method:: length()

    Return the length of the vector.

  .. method:: length2()

    Return the length of the vector squared.

  .. method:: distance(vec)

    Return the distance to another vector.
    Equivalent to ``(self-vec).length()``.

  .. method:: distance2(vec)

    Return the distance to another vector squared.
    Equivalent to ``(self-vec).length2()``.

  .. method:: normalize()

    Normalize the vector, such as *x² + y² = 1* (for :class:`vec2`) or
    *x² + y² + z² = 1* (for :class:`vec3`).

  .. method:: rotate([axis,] angle)

    Rotate the vector. *axis* must be provided (only) for :class:`vec3`.

  .. method:: angle(vec)

    Return the angle with another vector.

  .. method:: cross(vec)

    *(Only for vec3.)* Return the cross product with another vector.

  .. method:: triple(v1, v2)

    *(Only for vec3.)* Return the scalar triple product of this vector with two others.

  .. method:: self == other
              self != other

    Vector comparison.

  .. method:: self + other
              self - other
              -self
              k * self
              self * k
              self / k
              __abs__()

    Basic vector operations.

  .. method:: self * vec

    Return the element-wise product of two vectors.

  .. method:: __iter__()

    Return an iterator on vector coordinates.


Quaternions --- :class:`quat`
-----------------------------

.. class:: quat(x=0, y=0, z=0, w=1)
           quat(axis, angle=0)
           quat(yaw=0, pitch=0, roll=0)

  Return a new quaternion built from *x*, *y*, *z*, *w* components, an
  axis and a rotation around it, or Euler angles.

  Arguments are scalar excepting *axis* which is a 3D vector.

  .. note:: When called with 1 to 3 unnamed arguments, the Euler angles form is used.

  .. attribute:: x
                 y
                 z
                 w

    Quaternion components.

  .. attribute:: axis

    Axis of rotation represented by the quaternion.

  .. attribute:: angle

    Angle of rotation represented by the quaternion.

  .. method:: dot(q)

    Return the dot product with another quaternion.

  .. method:: length()

    Return the length of the quaternion.

  .. method:: length2()

    Return the length of the quaternion squared.

  .. method:: normalize()

    Normalize the quaternion, such as *x² + y² + z² + w² = 1*.

  .. method:: angle_with(q)

    Return the angle with another quaternion.

  .. method:: inverse()

    Return the inverse of the quaternion.

  .. todo::
  
    .. method:: farthest(q)
    .. method:: nearest(q)
    .. method:: rotate(vec)

  .. method:: self == other
              self != other

    Quaternion comparison.

  .. method:: self + other
              self - other
              -self
              self * k
              self / k
              __abs__()
              self * other
              vec * self
              self * vec

    Basic quaternion operations.

  .. method:: __iter__()

    Return an iterator on quaternion coordinates.


Rotation matrices --- :class:`matrix3`
--------------------------------------

.. class:: matrix3(yaw=0, pitch=0, roll=0)
           matrix3(row0, row1, row2)
           matrix3(xx, xy, xz, yx, yy, yz, zx, zy, zz)

  Return a new 3x3 rotation matrix built from Euler angles, rows or matrix
  terms. If no argument is provided, return the identity matrix.

  .. method:: row(i)

    Return matrix's row *i* as a :class:`vec3` (0 ≤ *i* ≤ 2).

  .. method:: col(i)

    Return matrix's column *i* as a :class:`vec3` (0 ≤ *i* ≤ 2).

  .. method:: self[i]

    Equivalent to ``self.row(i)``.

  .. attribute:: euler_ypr

    Euler angles around YXZ represented by the matrix, as a 3-uple.

  .. attribute:: euler_zyx

    Euler angles around ZYX represented by the matrix, as a 3-uple.

  .. method:: scale(vec)

    Scale columns of a matrix using vector components.

  .. method:: determinant()

    Return the determinant of the matrix.

  .. method:: adjoint()

    Return the adjoint of the matrix.

  .. method:: transpose()

    Return the transpose of the matrix.

  .. method:: transpose_times(other)

    Equivalent to ``self.transpose() * other``.

  .. method:: times_transpose(other)

    Equivalent to ``self * other.transpose()``.

  .. method:: inverse()

    Return the inverse of the matrix.

  .. method:: tdotx(vec)
              tdoty(vec)
              tdotz(vec)

    Return the dot product between *x*, *y* or *z* column and *vec*.

  .. staticmethod:: cofac(r1, c1, r2, c2)

    Return the cofactor of the matrix.

  .. method:: self == other

    Matrix comparison.

  .. method:: self * other
              __abs__()
              self * vec
              vec * self

    Basic matrix operations.

  .. method:: __iter__()

    Return an iterator on matrix rows, as vectors.


Transformations --- :class:`trans`
----------------------------------

A 3D transformation combine a position (:class:`vec3`) and a rotation
(:class:`quat` or :class:`matrix3`).

.. class:: trans(rot, origin=vec3())
           trans(basis=matrix3(), origin=vec3())
           trans(origin)

  Return a new transformation. *origin* is a :class:`vec3`, *rot* is a
  :class:`quat` and *basis* is a :class:`matrix3`.
  If no argument is provided, return the identity transformation.

  .. attribute:: origin

    The origin vector, as a :class:`vec3`.

  .. attribute:: basis

    The basis matrix for the rotation, as a :class:`matrix3`.

  .. attribute:: rotation

    The rotation, as a :class:`quat`.

  .. method:: inverse()

    Return the inverse of the transformation.

  .. method:: inverse_times(other)

    Equivalent to ``self.inverse() * other``.

  .. method:: self == other

    Transformation comparison.

  .. method:: self * other
              self * vec
              self * quat

    Basic transformation operations.


