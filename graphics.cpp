#include <GL/glut.h>
#include "graphics.h"

namespace graphics {


void drawCylinder(btScalar r, btScalar h, unsigned int slices, btScalar start, btScalar sweep)
{
  glBegin(GL_QUAD_STRIP);
  for(unsigned int i=0; i<=slices; ++i) {
    btScalar angle = i*sweep/slices + start;
    btScalar vcos = btCos(angle);
    btScalar vsin = btSin(angle);
    btglNormal3(vcos, vsin, 0);
    btglVertex3(vcos*r, vsin*r, 0);
    btglVertex3(vcos*r, vsin*r, h);
  }
  glEnd();
}

void drawDisk(btScalar r0, btScalar r1, btScalar z, unsigned int slices, btScalar start, btScalar sweep)
{
  btglNormal3(0, 0, 1);
  if(r0 == 0) {
    glBegin(GL_TRIANGLE_FAN);
    for(unsigned int i=0; i<=slices; ++i) {
      btScalar angle = i*sweep/slices + start;
      btScalar vcos = btCos(angle);
      btScalar vsin = btSin(angle);
      btglVertex3(vcos*r1, vsin*r1, z);
    }
    glEnd();
  } else {
    glBegin(GL_QUAD_STRIP);
    for(unsigned int i=0; i<=slices; ++i) {
      btScalar angle = i*sweep/slices + start;
      btScalar vcos = btCos(angle);
      btScalar vsin = btSin(angle);
      btglVertex3(vcos*r0, vsin*r0, z);
      btglVertex3(vcos*r1, vsin*r1, z);
    }
    glEnd();
  }
}

void drawClosedCylinder(btScalar r, btScalar h, unsigned int slices)
{
  // precompute cos/sin values
  btScalar vcos[slices+1];
  btScalar vsin[slices+1];
  btScalar a = 2*M_PI/slices;
  for(unsigned int i=0; i<=slices; ++i) {
    vcos[i] = btCos(i*a);
    vsin[i] = btSin(i*a);
  }

  glBegin(GL_QUAD_STRIP);
  for(unsigned int i=0; i<=slices; ++i) {
    btglNormal3(vcos[i], vsin[i], 0);
    btglVertex3(vcos[i]*r, vsin[i]*r, 0);
    btglVertex3(vcos[i]*r, vsin[i]*r, h);
  }
  glEnd();

  btglNormal3(0, 0, -1);
  glBegin(GL_TRIANGLE_FAN);
  for(unsigned int i=0; i<=slices; ++i) {
    btglVertex3(vcos[i]*r, vsin[i]*r, 0);
  }
  glEnd();

  btglNormal3(0, 0, 1);
  glBegin(GL_TRIANGLE_FAN);
  for(unsigned int i=0; i<=slices; ++i) {
    btglVertex3(vcos[i]*r, vsin[i]*r, h);
  }
  glEnd();
}


}
