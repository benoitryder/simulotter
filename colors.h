#ifndef COLORS_H
#define COLORS_H

#include <GL/gl.h>

///@file

typedef GLfloat Color4[4];

/// Fill a color from RGBA float values
#define COLOR_FILL4(dst,r,g,b,a) \
  (dst)[0] = (r), \
  (dst)[1] = (g), \
  (dst)[2] = (b), \
  (dst)[3] = (a)

/// Fill a color from RGB float values
#define COLOR_FILL3(dst,r,g,b) COLOR_FILL4(dst,r,g,b,0.0f)

/// Copy a color
#define COLOR_COPY(dst,src) \
  (dst)[0] = (src)[0], \
  (dst)[1] = (src)[1], \
  (dst)[2] = (src)[2], \
  (dst)[3] = (src)[3]

/// Create a float color array from byte values
#define COLOR_I2F(r,g,b)  {(r)/255.f, (g)/255.f, (b)/255.f, 1.0f}

#define COLOR_WHITE  {1.0f, 1.0f, 1.0f, 1.0f}
#define COLOR_BLACK  {0.0f, 0.0f, 0.0f, 1.0f}

#define COLOR_GRAY(n) {(GLfloat)(n), (GLfloat)(n), (GLfloat)(n), 1.0f}

#define COLOR_PLEXI {.70f, .90f, .95f, 0.5f}

#define COLOR_RAL_6018  COLOR_I2F(0x4f, 0xa8, 0x33)  // yellow green
#define COLOR_RAL_3020  COLOR_I2F(0xc7, 0x17, 0x12)  // traffic red
#define COLOR_RAL_5015  COLOR_I2F(0x17, 0x61, 0xab)  // sky blue
#define COLOR_RAL_8017  COLOR_I2F(0x2e, 0x1c, 0x1c)  // chocolate brown
/*
#define COLOR_RAL_6018  COLOR_I2F(0x0a, 0xa3, 0x02)  // yellow green
#define COLOR_RAL_3020  COLOR_I2F(0xd3, 0x01, 0x00)  // traffic red
#define COLOR_RAL_5015  COLOR_I2F(0x00, 0x60, 0xb1)  // sky blue
#define COLOR_RAL_8017  COLOR_I2F(0x0b, 0x00, 0x00)  // chocolate brown
*/


#endif
