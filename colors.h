#ifndef COLORS_H_
#define COLORS_H_

///@file

#include <GL/gl.h>
#include "maths.h"

class Color4
{
  friend Color4 operator*(const Color4& c, const GLfloat& f);
 private:
  GLfloat rgba_[4];
 public:

  constexpr Color4(): Color4(0.f, 0.f, 0.f, 1.f) {}
  constexpr Color4(GLfloat r, GLfloat g, GLfloat b, GLfloat a=1.0):
      rgba_{
        CLAMP(r, 0.0f, 1.0f),
        CLAMP(g, 0.0f, 1.0f),
        CLAMP(b, 0.0f, 1.0f),
        CLAMP(a, 0.0f, 1.0f),
      } {}
  constexpr Color4(int r, int g, int b, int a=255): Color4(r/255.f,g/255.f,b/255.f,a/255.f) {}
  constexpr Color4(GLfloat gray): Color4(gray,gray,gray,1.0) {}

  static const Color4 white;
  static const Color4 black;
  static const Color4 plexi;

  inline operator const GLfloat* () const { return rgba_; }
  inline GLfloat r() const { return rgba_[0]; }
  inline GLfloat g() const { return rgba_[1]; }
  inline GLfloat b() const { return rgba_[2]; }
  inline GLfloat a() const { return rgba_[3]; }

  //inline GLfloat operator[](int i) { return rgba_[i]; }
};

inline Color4 operator*(const Color4& c, const GLfloat& f)
{
  return Color4(c.rgba_[0]*f, c.rgba_[1]*f, c.rgba_[2]*f, c.rgba_[3]*f);
}
inline Color4 operator*(const GLfloat& f, const Color4& c) { return c*f; }


#endif
