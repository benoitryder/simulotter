#ifndef BULLET_H_
#define BULLET_H_

/** @file
 * @brief Bullet related declarations
 */

#include <btBulletDynamicsCommon.h>

// check bullet version
#if BT_BULLET_VERSION < 278
#error "Bullet >= 2.78 is required"
#endif


/** @name GL aliases to use Bullet float precision
 */
//@{
#if defined(BT_USE_DOUBLE_PRECISION)
#define btglLoadMatrix glLoadMatrixd
#define btglMultMatrix glMultMatrixd
#define btglVertex3    glVertex3d
#define btglNormal3    glNormal3d
#define btglScale      glScaled
#define btglTranslate  glTranslated
#define btglRotate     glRotated
#define btglRect       glRectd
#else
#define btglLoadMatrix glLoadMatrixf
#define btglMultMatrix glMultMatrixf
#define btglVertex3    glVertex3f
#define btglNormal3    glNormal3f
#define btglScale      glScalef
#define btglTranslate  glTranslatef
#define btglRotate     glRotatef
#define btglRect       glRectf
#endif
//@}


/** @brief Bullet scaling factor
 *
 * Bullet does not work well with too small or too big objects, thus lengths
 * are scaled.
 *
 * @sa http://www.bulletphysics.com/mediawiki-1.5.8/index.php?title=Scaling_The_World
 */
#define BULLET_SCALE   0.1

/// Literal for values in user unit (m is for meter)
constexpr inline long double operator "" _m(long double v) { return v / BULLET_SCALE; }
/// Literal for values in user unit, float alternative
constexpr inline float operator "" _mf(long double v) { return v / BULLET_SCALE; }

/// Scale from user units to Bullet units
template<typename T> T btScale(const T& t) { return t / BULLET_SCALE; }

/// Scale from Bullet units to user units
template<typename T> T btUnscale(const T& t) { return t * BULLET_SCALE; }

template<> inline btTransform btScale(const btTransform& tr) { return btTransform(tr.getBasis(),btScale(tr.getOrigin())); }
template<> inline btTransform btUnscale(const btTransform& tr) { return btTransform(tr.getBasis(),btUnscale(tr.getOrigin())); }

#undef BULLET_SCALE

#endif
