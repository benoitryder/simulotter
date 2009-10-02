#ifndef PHYSICS_H
#define PHYSICS_H

#include <set>
#include <queue>
#include <vector>
#include "global.h"

class Object;
class TaskPhysics;


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
  bool isInitialized() const { return step_dt > 0; }

  /// Advance simulation
  void step();

  /// Return current simulation time
  btScalar getTime() const { return time; }

  /** @brief Schedule a task
   *
   * If \e time is negative, the task will be executed after the next
   * step.
   *
   * @note Precision of execution time will depends on simulation time step.
   * Execution order is not affected.
   */
  void scheduleTask(TaskPhysics *task, btScalar time=-1);

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

  btScalar time;

  typedef std::pair< btScalar, SmartPtr<TaskPhysics> > TaskQueueValue;
  typedef std::priority_queue< TaskQueueValue, std::vector<TaskQueueValue>, std::greater<TaskQueueValue> > TaskQueue;
  /// Scheduled tasks
  TaskQueue task_queue;
};


/** @brief Scheduled task interface
 *
 * Parent class for tasks scheduled at a given simulation time.
 */
class TaskPhysics: public SmartObject
{
public:
  TaskPhysics() {}
  virtual ~TaskPhysics() {}

  virtual void process(Physics *ph) = 0;
};

/** @brief Basic task
 */
class TaskBasic: public TaskPhysics
{
public:
  TaskBasic(btScalar period=0.0);
  virtual ~TaskBasic() {}

  virtual void process(Physics *ph);

  /// Cancel the task
  void cancel() { cancelled = true; }

  typedef void (*Callback)(Physics *ph);
  void setCallback(Callback cb) { callback = cb; }

protected:
  btScalar period; /// Period for repeated tasks (or 0)
  Callback callback;
  bool cancelled;
};

/** @brief Task used in Lua
 *
 * Behavior is similar to \e TaskBasic's.
 *
 * Callback is stored on the instance \e callback field.
 */
class TaskLua: public TaskPhysics
{
public:
  TaskLua(int ref, btScalar period=0.0);
  virtual ~TaskLua();

  virtual void process(Physics *ph);

  /// Cancel the task
  void cancel() { cancelled = true; }

protected:
  btScalar period;
  bool cancelled;
  /// Lua instance reference
  int ref_obj;

private:
  /// Process callback function call 
  void do_process_function(lua_State *L);
  /// Process callback coroutine call 
  void do_process_thread(lua_State *L);
};


/** @brief Compound shape which hold reference on children.
 *
 * The default btCompoundShape does not allow to keep references on children.
 * This class fulfil this need, especially for the LUA bindings.
 *
 * The \e updateChildReferences must be called after changes on children.
 *
 * @note Since parent methods are not virtual, they cannot be overloaded and it
 * is not possible to make automatically update references. This makes this
 * class pretty unsafe. Be careful when using it.
 */
class CompoundShapeSmart: public btCompoundShape
{
public:
  CompoundShapeSmart(bool enableDynamicAabbTree=true):
    btCompoundShape(enableDynamicAabbTree) {}
  virtual ~CompoundShapeSmart();

  /// Update references using current child list
  void updateChildReferences();
  /// Release referenced children
  void clearChildReferences();

  virtual const char *getName() const { return "CompoundSmart"; }

private:
  /// Store a copy of each referenced child
  std::vector<btCollisionShape *> ref_children;
};


#endif
