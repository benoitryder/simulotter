#ifndef MATHS_H_
#define MATHS_H_

///@file

#include <LinearMath/btScalar.h>
#include <LinearMath/btVector3.h>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(v,a,b) (((v)<(a))?(a):((v)>(b)?(b):(v)))

#ifdef BT_USE_DOUBLE_PRECISION
inline btScalar btCeil(btScalar x) { return ceil(x); }
#else
inline btScalar btCeil(btScalar x) { return ceilf(x); }
#endif


/** @name 2D points and vectors
 *
 * Available functions are similar to btVector3 ones.
 * Values are stored in public x and y attributes for convenience.
 */
//@{
class btVector2
{
 public:
  btScalar xy_[2];

 public:
  inline btVector2() {}
  inline btVector2(const btScalar& x, const btScalar& y) { xy_[0] = x; xy_[1] = y; }
  inline btVector2(const btVector3& v3)
  {
    xy_[0] = v3.x();
    xy_[1] = v3.y();
  }
  inline operator btVector3() const { return btVector3(xy_[0], xy_[1], 0); }

  inline operator const btScalar* () const { return xy_; }
  inline const btScalar& x() const { return xy_[0]; }
  inline const btScalar& y() const { return xy_[1]; }
  inline void setX(const btScalar& x) { xy_[0] = x; }
  inline void setY(const btScalar& y) { xy_[1] = y; }

  inline btVector2& operator+=(const btVector2& v) { xy_[0] += v.xy_[0]; xy_[1] += v.xy_[1]; return *this; }
  inline btVector2& operator-=(const btVector2& v) { xy_[0] -= v.xy_[0]; xy_[1] -= v.xy_[1]; return *this; }
  inline btVector2& operator*=(const btVector2& v) { xy_[0] *= v.xy_[0]; xy_[1] *= v.xy_[1]; return *this; }
  inline btVector2& operator*=(const btScalar& s) { xy_[0] *= s; xy_[1] *= s; return *this; }
  inline btVector2& operator/=(const btScalar& s) { btFullAssert(s != btScalar(0.0)); return *this *= btScalar(1.0) / s; }
  inline btScalar dot(const btVector2& v) const { return xy_[0] * v.xy_[0] + xy_[1] * v.xy_[1]; }
  inline btScalar length2() const { return dot(*this); }
  inline btScalar length () const { return btSqrt(length2()); }
  inline btScalar distance2(const btVector2& v) const;
  inline btScalar distance (const btVector2& v) const;
  inline btVector2& normalize() { return *this /= length(); }
  inline btVector2 normalized() const;

  inline btVector2& rotate(const btScalar angle);
  inline btVector2 rotated(const btScalar angle) const;
  inline btVector2 absolute() const { return btVector2( btFabs(xy_[0]), btFabs(xy_[1]) ); }
  inline btScalar angle() const { return btAtan2(xy_[1], xy_[0]); }
};

inline btVector2 operator+(const btVector2& v1, const btVector2& v2) { return btVector2(v1.xy_[0]+v2.xy_[0], v1.xy_[1]+v2.xy_[1]); }
inline btVector2 operator-(const btVector2& v1, const btVector2& v2) { return btVector2(v1.xy_[0]-v2.xy_[0], v1.xy_[1]-v2.xy_[1]); }
inline btVector2 operator*(const btVector2& v1, const btVector2& v2) { return btVector2(v1.xy_[0]*v2.xy_[0], v1.xy_[1]*v2.xy_[1]); }
inline btVector2 operator-(const btVector2& v) { return btVector2(-v.xy_[0], -v.xy_[1]); }
inline btVector2 operator*(const btScalar& s, const btVector2& v) { return btVector2(s*v.xy_[0], s*v.xy_[1]); }
inline btVector2 operator*(const btVector2& v, const btScalar& s) { return s*v; }
inline btVector2 operator/(const btVector2& v, const btScalar& s) { btFullAssert(s != btScalar(0.0)); return v * (btScalar(1.0)/s); }
inline bool operator==(const btVector2& v1, const btVector2& v2) { return v1.xy_[0] == v2.xy_[0] && v1.xy_[1] == v2.xy_[1]; }
inline bool operator!=(const btVector2& v1, const btVector2& v2) { return !(v1 == v2); }

inline btScalar btVector2::distance2(const btVector2& v) const { return (v - *this).length2(); }
inline btScalar btVector2::distance (const btVector2& v) const { return (v - *this).length(); }
inline btVector2 btVector2::normalized() const { return *this / length(); }
inline btVector2& btVector2::rotate(const btScalar angle)
{
  btVector2 v(-xy_[1], xy_[0]);
  return *this = ( *this * btCos(angle) + v * btSin(angle) );
}
inline btVector2 btVector2::rotated(const btScalar angle) const
{
  btVector2 v(-xy_[1], xy_[0]);
  return ( *this * btCos(angle) + v * btSin(angle) );
}

inline btScalar dot(const btVector2& v1, const btVector2& v2) { return v1.dot(v2); }
inline btScalar distance2(const btVector2& v1, const btVector2& v2) { return v1.distance2(v2); }
inline btScalar distance (const btVector2& v1, const btVector2& v2) { return v1.distance(v2); }
//@}


#endif
