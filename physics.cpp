#include <cstring>
#include "physics.h"
#include "object.h"
#include "log.h"


btScalar Physics::world_gravity = btScale(9.80665);
btScalar Physics::margin_epsilon = btScale(0.001);
btVector3 Physics::world_aabb_min = btScale(btVector3(-10,-5,-2));
btVector3 Physics::world_aabb_max = btScale(btVector3(10,5,2));
unsigned int Physics::world_objects_max = 300;


Physics::Physics(btScalar step_dt): pause_state_(false), step_dt_(0), time_(0)
{
  if( step_dt <= 0 )
    throw(Error("invalid step_dt value"));
  step_dt_ = step_dt;

  col_config_ = new btDefaultCollisionConfiguration();
  dispatcher_ = new btCollisionDispatcher(col_config_);
  broadphase_ = new btAxisSweep3(world_aabb_min, world_aabb_max, world_objects_max);
  solver_ = new btSequentialImpulseConstraintSolver();

  world_ = new btDiscreteDynamicsWorld(
      dispatcher_, broadphase_, solver_, col_config_
      );
  world_->setGravity(btVector3(0,0,-world_gravity));
  world_->setInternalTickCallback(worldTickCallback, this);
}

Physics::~Physics()
{
  tick_objs_.clear();

  // removeFromWorld() modify the set, don't use an iterator
  while( !objs_.empty() )
  {
    // keep a reference to avoid deleting the objing during the call
    SmartPtr<Object> obj = *objs_.begin();
    obj->removeFromWorld();
  }
  delete world_;
  delete solver_;
  delete dispatcher_;
  delete col_config_;
  delete broadphase_;
}


void Physics::step()
{
  if( pause_state_ )
    return;

  //XXX Simulation goes smoother with several 1-substep calls than with 1
  // several-substep-call. Yes, it's a bit strange.
  world_->stepSimulation(step_dt_, 0, step_dt_);
  time_ += step_dt_;

  // Scheduled tasks
  while( !task_queue_.empty() && task_queue_.top().first <= time_ )
  {
    // the task may push other tasks
    // popping after processing may pop one of these tasks
    SmartPtr<TaskPhysics> task = task_queue_.top().second;
    task_queue_.pop();
    task->process(this);
  }
}


void Physics::scheduleTask(TaskPhysics *task, btScalar time)
{
  task_queue_.push( TaskQueueValue(time, SmartPtr<TaskPhysics>(task)) );
}

void Physics::worldTickCallback(btDynamicsWorld *world, btScalar /*step*/)
{
  Physics *physics = (Physics*)world->getWorldUserInfo();

  std::set< SmartPtr<Object> >::const_iterator it_obj;
  for( it_obj = physics->tick_objs_.begin(); it_obj != physics->tick_objs_.end(); ++it_obj )
    (*it_obj)->tickCallback();
}


const btRigidBody Physics::static_body( btRigidBody::btRigidBodyConstructionInfo(0,NULL,NULL) );



TaskBasic::TaskBasic(btScalar period):
  period_(period), callback_(NULL), cancelled_(false)
{
}

void TaskBasic::process(Physics *ph)
{
  if( cancelled_ )
    return;
  if( callback_ == NULL )
    throw(Error("TaskBasic::process(): no callback"));

  callback_(ph);
  if( period_ > 0.0 )
    ph->scheduleTask(this, ph->getTime() + period_);
}


CompoundShapeSmart::~CompoundShapeSmart()
{
  clearChildReferences();
}

void CompoundShapeSmart::updateChildReferences()
{
  clearChildReferences();
  int n = getNumChildShapes();
  ref_children_.reserve(n);
  for( int i=0; i<n; i++ )
  {
    btCollisionShape *sh = getChildShape(i);
    SmartPtr_add_ref(sh);
    ref_children_.push_back(sh);
  }
}

void CompoundShapeSmart::clearChildReferences()
{
  std::vector<btCollisionShape *>::iterator it;
  for( it=ref_children_.begin(); it!=ref_children_.end(); ++it )
    SmartPtr_release( (*it) );
  ref_children_.clear();
}


