#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include "global.h"


///@file


/** @brief Physics environment
 */
class Physics
{
public:
  Physics();
  ~Physics();

  /// Init physics (using configuration values)
  void init();

  /// Advance simulation and update robots
  void step();

  void pause()   { this->pause_state = true;  }
  void unpause() { this->pause_state = false; }
  void togglePause() { this->pause_state = !this->pause_state; }

  btDynamicsWorld *getWorld() { return this->world; }

  /** @brief Add an object to the simulation
   *
   * The function will fail if the object is not initialized.
   *
   * @todo Allow to use <tt>addRigidBody(body,mask,mask)</tt>
   */
  void addObject(Object *obj);

  std::vector<Object*> &getObjs() { return this->objs; }

  /// Common static rigid body for constraints
  static btRigidBody static_body;

private:
  btDynamicsWorld          *world;
  btDispatcher             *dispatcher;
  btConstraintSolver       *solver;
  btBroadphaseInterface    *broadphase;
  btCollisionConfiguration *col_config;

  /// All match objects
  std::vector<Object*> objs;

  /// Simulation pause state
  bool pause_state;

  /// Simulation time step, must not be modified
  btScalar step_dt;
};


#endif
