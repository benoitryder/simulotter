#include <math.h>
#include <ode/ode.h>
#include "maths.h"


dReal norma(dReal a)
{
  a = fmod(a, 2*M_PI);
  if( a < -M_PI )
    a += 2*M_PI;
  else if( a >= M_PI )
    a -= 2*M_PI;
  return a;
}

dReal absf(dReal a)
{
  return ( a < 0.0 ) ? -a : a;
}

dReal dist2d(dReal x1, dReal y1, dReal x2, dReal y2)
{
	return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

dReal dist3d(dReal x1, dReal y1, dReal x2, dReal y2, dReal z1, dReal z2)
{
	return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) );
}

dReal signf(dReal a)
{
  return ( a < 0.0 ) ? -1.0 : 1.0;
}

void spheric2cart(float cart[3], float r, float theta, float phi)
{
  cart[0] = r * sin(theta) * cos(phi);
  cart[1] = r * sin(theta) * sin(phi);
  cart[2] = r * cos(theta);
}

void cart2spheric(float spheric[3], float x, float y, float z)
{
  spheric[0] = sqrt(x*x+y*y+z*z);
  spheric[1] = ( z == 0 ) ? M_PI_2 : atan2(sqrt(x*x+y*y), z);
  spheric[2] = ( x == 0 ) ? M_PI_2 : atan2(y, x);
}


void spheric2cart_add(float cart[3], float r, float theta, float phi)
{
  cart[0] += r * sin(theta) * cos(phi);
  cart[1] += r * sin(theta) * sin(phi);
  cart[2] += r * cos(theta);
}

