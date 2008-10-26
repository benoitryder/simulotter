#ifndef PHYSICS_H
#define PHYSICS_H

#include <ode/ode.h>
#include <vector>
#include "object.h"

///@file


/** @brief Physics environment
 */
class Physics
{
public:
  Physics();
  ~Physics();

  /// Advance simulation and update robots
  void step();

  void pause()   { this->pause_state = true;  }
  void unpause() { this->pause_state = false; }
  void toggle_pause() { this->pause_state = !this->pause_state; }

  dSpaceID get_space() const { return this->space; }
  dWorldID get_world() const { return this->world; }
  dJointGroupID get_joints() const { return this->joints; }

  std::vector<Object*> &get_objs() { return this->objs; }

private:
  dWorldID world;
  dSpaceID space;
  dJointGroupID joints;

  std::vector<Object*> objs;

  /// Simulation pause state
  bool pause_state;

  /// Function called by ODE during collision detection
  static void collide_callback(void *data, dGeomID o1, dGeomID o2);

  /// Geom pair, for cylinder hack
  typedef struct { dGeomID o1, o2; } GeomPair;

  /** @brief Cylinder-cylinder collisions to test
   *
   * Space cannot be modified in the callback function.
   * Thus, we store hacked collisions to process them later.
   */
  std::vector<GeomPair> hack_cylinders;
public:
  std::vector<dGeomID>hack_boxes;///XXX-hackboxes
};


/** @brief Collision categories
 *
 * Categories are bitfields used to determine which objects may collide.
 */
enum
{
  CAT_DYNAMIC   =   0x1,  ///< Dynamic object
  CAT_GROUND    =   0x2,  ///< Table ground
  CAT_ROBOT     =   0x5,  ///< Robot (dynamic)
  CAT_ELEMENT   =   0x9,  ///< Playing element (dynamic)
  CAT_DISPENSER =  0x10,  ///< Vertical dispenser in which elements fall (static)

};


#endif

