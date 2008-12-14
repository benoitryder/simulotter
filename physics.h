#ifndef PHYSICS_H
#define PHYSICS_H

class Physics;

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

  /** @brief Utility function to duplicate a geom
   *
   * Only geom parameters are copied: class and associated parameters, position
   * and rotation.
   * The returned geom is not attached to a body (thus has no offset) and is
   * not in any space.
   */
  static dGeomID geom_duplicate(dGeomID geom);

  std::vector<Object*> &get_objs() { return this->objs; }

private:
  dWorldID world;
  dSpaceID space;
  dJointGroupID joints;

  /// All match objects
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
 *
 * @note Category and collide bits used by ODE are <tt>long</tt>s. We use
 * <tt>int</tt>s but it is not a problem.
 */
enum
{
  CAT_NONE      =   0x0,
  /// Dynamic object
  CAT_DYNAMIC   =   0x1,
  /// Table ground
  CAT_GROUND    =   0x2,
  /// Robot (dynamic)
  CAT_ROBOT     =   0x4|CAT_DYNAMIC,
  /// Playing element (dynamic)
  CAT_ELEMENT   =   0x8|CAT_DYNAMIC,
  /// Use a custom collision function
  CAT_HANDLER   =  0x10,
  /// Temporary geom for cylinder hack
  CAT_CYLINDER_HACK = (1<<31),

  CAT_ALL       = -1
};


#endif
