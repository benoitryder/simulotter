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

  Color4() { set(0,0,0,1); }
  Color4(GLfloat r, GLfloat g, GLfloat b, GLfloat a=1.0) { set(r,g,b,a); }
  Color4(GLdouble r, GLdouble g, GLdouble b, GLdouble a=1.0) { set(r,g,b,a); }
  Color4(int r, int g, int b, int a=255) { set(r/255.f,g/255.f,b/255.f,a/255.f); }
  Color4(GLfloat gray) { set(gray,gray,gray,1.0); }

  void set(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
  {
    rgba_[0] = CLAMP(r, 0.0, 1.0);
    rgba_[1] = CLAMP(g, 0.0, 1.0);
    rgba_[2] = CLAMP(b, 0.0, 1.0);
    rgba_[3] = CLAMP(a, 0.0, 1.0);
  }

  static Color4 white() { return Color4(1.0); }
  static Color4 black() { return Color4(0.0); }
  static Color4 plexi() { return Color4(.70, .90, .95, 0.5); }

  inline operator const GLfloat* () const { return rgba_; }
  inline GLfloat r() const { return rgba_[0]; }
  inline GLfloat g() const { return rgba_[1]; }
  inline GLfloat b() const { return rgba_[2]; }
  inline GLfloat a() const { return rgba_[3]; }

  //inline GLfloat operator[](int i) { return rgba_[i]; }

  inline const Color4& operator*=(const GLfloat& f)
  {
    set(rgba_[0]*f, rgba_[1]*f, rgba_[2]*f, rgba_[3]*f);
    return *this;
  }
};

inline Color4 operator*(const Color4& c, const GLfloat& f)
{
  return Color4(c.rgba_[0]*f, c.rgba_[1]*f, c.rgba_[2]*f, c.rgba_[3]*f);
}
inline Color4 operator*(const GLfloat& f, const Color4& c) { return c*f; }


#endif
