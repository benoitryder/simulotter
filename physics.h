#ifndef PHYSICS_H
#define PHYSICS_H

#include <set>
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

  std::set<Object*> &getObjs() { return this->objs; }

  /// Common static rigid body for constraints
  static btRigidBody static_body;

private:
  btDynamicsWorld          *world;
  btDispatcher             *dispatcher;
  btConstraintSolver       *solver;
  btBroadphaseInterface    *broadphase;
  btCollisionConfiguration *col_config;

  /// All match objects
  std::set<Object*> objs;

  /// Simulation pause state
  bool pause_state;

  /// Simulation time step, must not be modified
  btScalar step_dt;
};


#endif
