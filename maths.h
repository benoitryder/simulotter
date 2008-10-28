#ifndef MATHS_H
#define MATHS_H

#include <ode/ode.h>

///@file

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


/// Normalize angle in [-Pi,Pi[
dReal norma(dReal a);

/// Absolute value
dReal absf(dReal a);

/// Distance between two 2D points
dReal dist2d(dReal x1, dReal y1, dReal x2, dReal y2);

/// Distance between two 3D points
dReal dist3d(dReal x1, dReal y1, dReal x2, dReal y2, dReal z1, dReal z2);

/// Sign (-1 if a<0, 1 otherwise)
dReal signf(dReal a);

/// Convert degrees to radians
#define DEG2RAD(a) (M_PI*(a)/180.0)

/// Convert radians to degrees
#define RAD2DEG(a) (180.0*(a)/M_PI)

/// Spherical coordinates to cartesian
void spheric2cart(float cart[3], float r, float theta, float phi);

/// Cartesian coordinates to spherical
void cart2spheric(float spheric[3], float x, float y, float z);

/// Add spherical coordinates to cartesian
void spheric2cart_add(float cart[3], float r, float theta, float phi);


#endif
