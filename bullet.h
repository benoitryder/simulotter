#ifndef BULLET_H
#define BULLET_H

#include <btBulletDynamicsCommon.h>

/** @file
 * @brief Bullet related declarations.
 */


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
#else
#define btglLoadMatrix glLoadMatrixf
#define btglMultMatrix glMultMatrixf
#define btglVertex3    glVertex3f
#define btglNormal3    glNormal3f
#define btglScale      glScalef
#define btglTranslate  glTranslatef
#define btglRotate     glRotatef
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


/// Scale from user units to Bullet units
template<typename T> T scale(T t) { return t / BULLET_SCALE; }

/// Scale from Bullet units to user units
template<typename T> T unscale(T t) { return t * BULLET_SCALE; }

#endif
