#ifndef MATHS_H
#define MATHS_H

#include <LinearMath/btScalar.h>


///@file

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#ifdef BT_USE_DOUBLE_PRECISION
inline btScalar btFmod(btScalar x, btScalar y) { return fmod(x,y);  }
inline btScalar btCeil(btScalar x) { return ceil(x); }
#else
inline btScalar btFmod(btScalar x, btScalar y) { return fmodf(x,y);  }
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
  btScalar x, y;

public:
  inline btVector2() {}
  inline btVector2(const btScalar &x, const btScalar &y): x(x), y(y) {}
  inline btVector2(const btVector3 &v3)
  {
    x = v3.x();
    y = v3.y();
  }
  inline operator btVector3() const { return btVector3(x,y,0); }

  inline btVector2 &operator+=(const btVector2 &v) { x += v.x; y += v.y; return *this; }
  inline btVector2 &operator-=(const btVector2 &v) { x -= v.x; y -= v.y; return *this; }
  inline btVector2 &operator*=(const btScalar &s) { x *= s; y *= s; return *this; }
  inline btVector2 &operator/=(const btScalar &s) { btFullAssert(s != btScalar(0.0)); return *this *= btScalar(1.0) / s; }
  inline btScalar dot(const btVector2 &v) const { return x * v.x + y * v.y; }
  inline btScalar length2() const { return dot(*this); }
  inline btScalar length () const { return btSqrt(length2()); }
  inline btScalar distance2(const btVector2 &v) const;
  inline btScalar distance (const btVector2 &v) const;
  inline btVector2 &normalize() { return *this /= length(); }
  inline btVector2 normalized() const;

  inline btVector2 rotate(const btScalar angle);
  inline btVector2 absolute() const { return btVector2( btFabs(x), btFabs(y) ); }
  inline btScalar angle() const { return btAtan2(y,x); }
};

inline btVector2 operator+(const btVector2 &v1, const btVector2 &v2) { return btVector2(v1.x+v2.x,v1.y+v2.y); }
inline btVector2 operator-(const btVector2 &v1, const btVector2 &v2) { return btVector2(v1.x-v2.x,v1.y-v2.y); }
inline btVector2 operator-(const btVector2 &v) { return btVector2(-v.x,-v.y); }
inline btVector2 operator*(const btScalar &s, const btVector2 &v) { return btVector2(s*v.x,s*v.y); }
inline btVector2 operator*(const btVector2 &v, const btScalar &s) { return s*v; }
inline btVector2 operator/(const btVector2 &v, const btScalar &s) { btFullAssert(s != btScalar(0.0)); return v * (btScalar(1.0)/s); }
inline bool operator==(const btVector2 &v1, const btVector2 &v2) { return v1.x == v2.x && v1.y == v2.y; }

inline btScalar btVector2::distance2(const btVector2 &v) const { return (v - *this).length2(); }
inline btScalar btVector2::distance (const btVector2 &v) const { return (v - *this).length(); }
inline btVector2 btVector2::normalized() const { return *this / length(); }
inline btVector2 btVector2::rotate(const btScalar angle)
{
  btVector2 v(-y,x);
  return ( *this * btCos(angle) + v * btSin(angle) );
}

inline btScalar dot(const btVector2 &v1, const btVector2 &v2) { return v1.dot(v2); }
inline btScalar distance2(const btVector2 &v1, const btVector2 &v2) { return v1.distance2(v2); }
inline btScalar distance (const btVector2 &v1, const btVector2 &v2) { return v1.distance(v2); }
//@}



/** @name 3D points and vectors in spherical coordinates
 */
//@{
class btSpheric3
{
public:
  btScalar r, theta, phi;

public:
  inline btSpheric3() {}
  inline btSpheric3(const btScalar &r, const btScalar &theta, const btScalar &phi):
    r(r), theta(theta), phi(phi)
  {
    btFullAssert(r >= btScalar(0.0));
  }
  inline btSpheric3(const btVector3 &v)
  {
    btVector2 v2(v);
    r = v.length();
    theta = btAtan2( v2.length(), v.z());
    phi   = v2.angle();
  }

  inline operator btVector3() const
  {
    return r * btVector3(
        btSin(theta) * btCos(phi),
        btSin(theta) * btSin(phi),
        btCos(theta)
        );
  }

  inline btSpheric3 &operator*=(const btScalar &s)
  {
    r *= btFabs(s);
    if( s < 0 )
      theta += SIMD_PI;
    return *this;
  }
  inline btSpheric3 &operator/=(const btScalar &s) { btFullAssert(s != btScalar(0.0)); return *this *= btScalar(1.0) / s; }

  /// Add given values to theta and phi
  inline void rotate(const btScalar &t, const btScalar &p) { theta+=t; phi+=p; }
};

inline btSpheric3 operator*(const btScalar &s, const btSpheric3 &v)
{
  return btSpheric3(v.r*btFabs(s), btFsel(s,v.theta,v.theta+SIMD_PI), v.phi);
}
inline btSpheric3 operator*(const btSpheric3 &v, const btScalar &s) { return s*v; }
inline btSpheric3 operator/(const btSpheric3 &v, const btScalar &s) { btFullAssert(s != btScalar(0.0)); return v * (btScalar(1.0)/s); }

//@}


/// Normalize angle in [-Pi,Pi[
inline btScalar normA(btScalar a)
{
  a = btFmod(a, SIMD_2_PI);
  if( a < -SIMD_PI )
    a += SIMD_2_PI;
  else if( a >= SIMD_PI )
    a -= SIMD_2_PI;
  return a;
}


#endif
