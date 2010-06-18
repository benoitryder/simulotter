#include <cstring>
#include "physics.h"
#include "object.h"
#include "lua_utils.h"
#include "config.h"
#include "log.h"


SmartPtr<Physics> Physics::physics;

const btScalar Physics::EARTH_GRAVITY = btScale(9.80665);
const btScalar Physics::MARGIN_EPSILON = btScale(0.001);


Physics::Physics(btScalar step_dt): pause_state_(false), step_dt_(0), time_(0)
{
  if( step_dt <= 0 )
    throw(Error("invalid step_dt value"));
  step_dt_ = step_dt;

  col_config_ = new btDefaultCollisionConfiguration();
  dispatcher_ = new btCollisionDispatcher(col_config_);

  //XXX set world size and max object number
  broadphase_ = new btAxisSweep3(
      btScale(btVector3(-10,-5,-2)),
      btScale(btVector3(10,5,2)),
      300
      );

  solver_ = new btSequentialImpulseConstraintSolver();

  world_ = new btDiscreteDynamicsWorld(
      dispatcher_, broadphase_, solver_, col_config_
      );
  world_->setGravity(btVector3(0,0,-EARTH_GRAVITY));
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

TaskLua::TaskLua(lua_State *L, int ref, btScalar period):
  period_(period), cancelled_(false), L_(L), ref_obj_(ref)
{
  if( ref_obj_ == LUA_NOREF || ref_obj_ == LUA_REFNIL )
    throw(LuaError(L_, "TaskLua: invalid object reference"));
}

TaskLua::~TaskLua()
{
  luaL_unref(L_, LUA_REGISTRYINDEX, ref_obj_);
}

void TaskLua::process(Physics *ph)
{
  if( cancelled_ )
    return;
  // Get the callback
  lua_rawgeti(L_, LUA_REGISTRYINDEX, ref_obj_);
  lua_remove(L_, -2);
  lua_getfield(L_, -1, "callback");
  if( lua_isfunction(L_, -1) )
    do_process_function(L_);
  else if( lua_isthread(L_, -1) )
    do_process_thread(L_);
  else
    throw(Error("TaskLua: invalid callback"));

  if( period_ > 0 && !cancelled_ )
    ph->scheduleTask(this, ph->getTime() + period_);
}

void TaskLua::do_process_function(lua_State *L)
{
  LuaManager::pcall(L, 0, 0);
}

void TaskLua::do_process_thread(lua_State *L)
{
  lua_State *L_cb = lua_tothread(L, -1);
  lua_pop(L,1);
  int ret = lua_resume(L_cb, 0);
  if( ret == 0 )
    cancelled_ = true; // coroutine returned
  else if( ret != LUA_YIELD )
    throw(LuaError(L_cb));
}


/** @brief Lua Physics class
 * The constructor is a factory and returns the singleton.
 */
class LuaPhysics: public LuaClass<Physics>
{
  static int _ctor(lua_State *L)
  {
    if( Physics::physics == NULL ) {
      if( lua_isnoneornil(L, 2) ) {
        Physics::physics = new Physics();
      } else {
        Physics::physics = new Physics(LARG_f(2));
      }
    }
    store_ptr(L, Physics::physics);
    return 0;
  }

  LUA_DEFINE_GET(get_step_dt, getStepDt);
  LUA_DEFINE_GET(get_time, getTime);
  LUA_DEFINE_SET0(pause, pause);
  LUA_DEFINE_SET0(unpause, unpause);
  LUA_DEFINE_SET0(toggle_pause, togglePause);


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(get_step_dt);
    LUA_CLASS_MEMBER(get_time);
    LUA_CLASS_MEMBER(pause);
    LUA_CLASS_MEMBER(unpause);
    LUA_CLASS_MEMBER(toggle_pause);
  }
};

LUA_REGISTER_BASE_CLASS(Physics);


/** @brief Lua Task class
 */
class LuaTask: public LuaClass<TaskLua>
{
  static int _ctor(lua_State *L)
  {
    btScalar period_ = 0.0;
    if( !lua_isnoneornil(L, 2) )
      period_ = LARG_f(2);

    data_ptr *ud = new_ptr(L);
    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    *ud = new TaskLua(L, ref, period_);
    SmartPtr_add_ref(*ud);
    return 0;
  }

  LUA_DEFINE_SET0(cancel, cancel);

  static int schedule(lua_State *L)
  {
    if( !Physics::physics )
      return luaL_error(L, "physics is not created, cannot schedule");
    SmartPtr<TaskLua> task = get_ptr(L,1);
    if( lua_isnoneornil(L, 2) )
      Physics::physics->scheduleTask(task);
    else
      Physics::physics->scheduleTask(task, LARG_f(2));
    return 0;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(cancel);
    LUA_CLASS_MEMBER(schedule);
  }
};

LUA_REGISTER_BASE_CLASS_NAME(LuaTask,TaskLua,"Task");



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


/** @brief Lua collision shapes
 *
 * Shape is not a usual class: instances are created using a specific method
 * for each shape, there are no constructor.
 * Instances are not tables with an \e _ud fields but userdata.
 * store_ptr() and get_ptr() are redefined for this purpose.
 */
class LuaShape: public LuaClass<btCollisionShape>
{
  static void store_ptr(lua_State *L, btCollisionShape *p)
  {
    // check that we use Class:method and not Class.method
    luaL_checktype(L, 1, LUA_TUSERDATA);
    data_ptr *ud = (data_ptr *)lua_newuserdata(L, sizeof(data_ptr));
    lua_getfield(L, LUA_REGISTRYINDEX, name_);
    lua_setmetatable(L, -2);

    *ud = p;
    // Increase ref count and store the pointer
    SmartPtr_add_ref(p);
  }

  static data_ptr get_ptr(lua_State *L)
  {
    return *(data_ptr *)luaL_checkudata(L, 1, name_);
  }

  static int _ctor(lua_State *L)
  {
    return luaL_error(L, "no Shape constructor, use creation methods");
  }

  static int compound(lua_State *L)
  {
    CompoundShapeSmart *cshape = new CompoundShapeSmart();

    // Get the Shape metatable to check type
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_REGISTRY_PREFIX "Shape"); //XXX
    int shape_mt = lua_gettop(L);

    lua_pushnil(L);
    while( lua_next(L, 2) != 0 )
    {
      if( !lua_istable(L, -1) || lua_objlen(L, -1) != 2 )
        return luaL_error(L, "invalid child element, pair expected");

      // shape
      lua_rawgeti(L, -1, 1);
      void *p = lua_touserdata(L, -1);
      if( p == NULL )
        throw(LuaError(L, "invalid shape"));
      btCollisionShape *sh = *(btCollisionShape **)p;
      if( sh == NULL || !lua_getmetatable(L, -1) || !lua_rawequal(L, -1, shape_mt) )
        throw(LuaError(L, "invalid shape"));
      lua_pop(L, 2);

      // transform
      btTransform tr;
      lua_rawgeti(L, -1, 2);
      if( LuaManager::totransform(L, -1, tr) != 0 )
        throw(LuaError(L, "invalid transform"));
      lua_pop(L, 1);

      cshape->addChildShape(tr, sh);
      lua_pop(L, 1);
    }

    lua_remove(L, shape_mt);

    cshape->updateChildReferences();
    store_ptr(L, cshape);
    return 1;
  }

  static int sphere(lua_State *L)
  {
    store_ptr(L, new btSphereShape(LARG_scaled(2)));
    return 1;
  }

  static int box(lua_State *L)
  {
    store_ptr(L, new btBoxShape( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) ));
    return 1;
  }

  static int capsuleX(lua_State *L)
  {
    store_ptr(L, new btCapsuleShapeX(LARG_scaled(2), LARG_scaled(3)));
    return 1;
  }
  static int capsuleY(lua_State *L)
  {
    store_ptr(L, new btCapsuleShape(LARG_scaled(2), LARG_scaled(3)));
    return 1;
  }
  static int capsuleZ(lua_State *L)
  {
    store_ptr(L, new btCapsuleShapeZ(LARG_scaled(2), LARG_scaled(3)));
    return 1;
  }

  static int cylinderX(lua_State *L)
  {
    store_ptr(L, new btCylinderShapeX( btVector3(LARG_scaled(3), LARG_scaled(2), LARG_scaled(2)) ));
    return 1;
  }
  static int cylinderY(lua_State *L)
  {
    store_ptr(L, new btCylinderShape( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(2)) ));
    return 1;
  }
  static int cylinderZ(lua_State *L)
  {
    store_ptr(L, new btCylinderShapeZ( btVector3(LARG_scaled(2), LARG_scaled(2), LARG_scaled(3)) ));
    return 1;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(compound);
    LUA_CLASS_MEMBER(sphere);
    LUA_CLASS_MEMBER(box);
    LUA_CLASS_MEMBER(capsuleX);
    LUA_CLASS_MEMBER(capsuleY);
    LUA_CLASS_MEMBER(capsuleZ);
    LUA_CLASS_MEMBER(cylinderX);
    LUA_CLASS_MEMBER(cylinderY);
    LUA_CLASS_MEMBER(cylinderZ);
  }
};

LUA_REGISTER_BASE_CLASS_NAME(LuaShape,btCollisionShape,"Shape");

