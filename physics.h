#ifndef PHYSICS_H_
#define PHYSICS_H_

///@file

#include <set>
#include <queue>
#include <vector>
#include <boost/function.hpp>
#include "smart.h"

class Object;
class TaskPhysics;



/** @brief Physics environment
 */
class Physics: public SmartObject
{
public:
  
  /** @name Configuration values.
   */
  //@{

  /// Gravity at the world's surface.
  static btScalar world_gravity;
  /** @brief Minimal margin distance.
   *
   * Used as gap distance between objects to ensure they do not overlap (e.g.
   * for object dropping height).
   */
  static btScalar margin_epsilon;

  /// AABB's minimum for new worlds.
  static btVector3 world_aabb_min;
  /// AABB's maximum for new worlds.
  static btVector3 world_aabb_max;
  /// Maximum object count for new worlds.
  static unsigned int world_objects_max;

  //@}

  Physics(btScalar step_dt=0.002);
  virtual ~Physics();

  /// Advance simulation
  void step();

  btScalar getStepDt() const { return step_dt_; }

  /// Return current simulation time
  btScalar getTime() const { return time_; }

  /** @brief Schedule a task
   *
   * If \e time is negative, the task will be executed after the next
   * step.
   *
   * @note Precision of execution time will depends on simulation time step.
   * Execution order is not affected.
   */
  void scheduleTask(TaskPhysics *task, btScalar time=-1);

  btDynamicsWorld *getWorld() { return world_; }
  const btDynamicsWorld *getWorld() const { return world_; }

  std::set<SmartPtr<Object> > &getObjs() { return objs_; }
  std::set<SmartPtr<Object> > &getTickObjs() { return tick_objs_; }

  /// Common static rigid body for constraints
  static const btRigidBody static_body;

private:
  /** @brief Encapsulated world.
   *
   * The world user info is set to the physics instance pointer.
   */
  btDynamicsWorld          *world_;
  btDispatcher             *dispatcher_;
  btConstraintSolver       *solver_;
  btBroadphaseInterface    *broadphase_;
  btCollisionConfiguration *col_config_;

  /// All simulated objects
  std::set<SmartPtr<Object> > objs_;

  /** @brief Object whose tick callback must be called.
   * @sa Object::tickCallback()
   */
  std::set<SmartPtr<Object> > tick_objs_;

  /// Tick callback called by Bullet.
  static void worldTickCallback(btDynamicsWorld *world, btScalar step);

  /// Simulation time step, must not be modified
  btScalar step_dt_;

  btScalar time_;

  typedef std::pair< btScalar, SmartPtr<TaskPhysics> > TaskQueueValue;
  typedef std::priority_queue< TaskQueueValue, std::vector<TaskQueueValue>, std::greater<TaskQueueValue> > TaskQueue;
  /// Scheduled tasks
  TaskQueue task_queue_;
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
  void cancel() { cancelled_ = true; }

  typedef boost::function<void (Physics *)> Callback;
  void setCallback(Callback cb) { callback_ = cb; }

protected:
  btScalar period_; /// Period for repeated tasks (or 0)
  Callback callback_;
  bool cancelled_;
};


/** @brief Compound shape which hold reference on children.
 *
 * The default btCompoundShape does not allow to keep references on children.
 * This class fulfil this need, especially for bindings.
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
  std::vector<btCollisionShape *> ref_children_;
};


#endif
