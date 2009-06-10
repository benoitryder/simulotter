#ifndef PHYSICS_H
#define PHYSICS_H

#include <set>
#include "global.h"

class Object;


///@file


/** @brief Physics environment
 */
class Physics: public SmartObject
{
public:
  Physics();
  virtual ~Physics();

  /// Init physics (using configuration values)
  void init();
  bool isInitialized() { return step_dt > 0; }

  /// Advance simulation and update robots
  void step();

  void pause()   { this->pause_state = true;  }
  void unpause() { this->pause_state = false; }
  void togglePause() { this->pause_state = !this->pause_state; }

  btDynamicsWorld *getWorld() { return this->world; }

  std::set<SmartPtr<Object> > &getObjs() { return this->objs; }

  /// Common static rigid body for constraints
  static btRigidBody static_body;

private:
  btDynamicsWorld          *world;
  btDispatcher             *dispatcher;
  btConstraintSolver       *solver;
  btBroadphaseInterface    *broadphase;
  btCollisionConfiguration *col_config;

  /// All simulateed objects
  std::set<SmartPtr<Object> > objs;

  /// Simulation pause state
  bool pause_state;

  /// Simulation time step, must not be modified
  btScalar step_dt;
};


#endif
