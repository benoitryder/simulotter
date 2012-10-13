#ifndef GRAPHICS_H_
#define GRAPHICS_H_

///@file

#include "bullet.h"

namespace graphics {

/** @brief Draw a partial cylinder
 *
 * Cylinder is centered around z axis, from z=0 to z=h.
 */
void drawCylinder(btScalar r, btScalar h, unsigned int slices, btScalar start=0, btScalar sweep=2*M_PI);

/// Draw a partial disk centered on the origin
void drawDisk(btScalar r0, btScalar r1, btScalar z, unsigned int slices, btScalar start=0, btScalar sweep=2*M_PI);

/// Draw a cylinder with bottom and bottom faces
void drawClosedCylinder(btScalar r, btScalar h, unsigned int slices);

}

#endif
