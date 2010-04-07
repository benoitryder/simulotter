#include <cstring>
#include "physics.h"
#include "object.h"
#include "lua_utils.h"
#include "config.h"
#include "log.h"


SmartPtr<Physics> Physics::physics;


Physics::Physics()
{
  col_config = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(col_config);

  //XXX set world size and max object number
  broadphase = new btAxisSweep3(
      scale(btVector3(-10,-5,-2)),
      scale(btVector3(10,5,2)),
      100
      );

  solver = new btSequentialImpulseConstraintSolver();

  world = new btDiscreteDynamicsWorld(
      dispatcher, broadphase, solver, col_config
      );
  world->setInternalTickCallback(worldTickCallback, this);

  pause_state = false;
  step_dt = 0;
  time = 0;
}

Physics::~Physics()
{
  tick_objs.clear();

  // removeFromWorld() modify the set, don't use an iterator
  while( ! objs.empty() )
  {
    // keep a reference to avoid deleting the objing during the call
    SmartPtr<Object> obj = *objs.begin();
    obj->removeFromWorld();
  }
  delete world;
  delete solver;
  delete dispatcher;
  delete col_config;
  delete broadphase;
}

void Physics::init()
{
  if( isInitialized() )
  {
    LOG("physics already initialized, init() call skipped");
    return;
  }
  if( cfg.step_dt <= 0 )
    throw(Error("invalid step_dt value for Physics initialization"));
  step_dt = cfg.step_dt;
  world->setGravity(btVector3(0,0,cfg.gravity_z));
}


void Physics::step()
{
  if( this->pause_state )
    return;

  //XXX Simulation goes smoother with several 1-substep calls than with 1
  // several-substep-call. Yes, it's a bit strange.
  world->stepSimulation(step_dt, 0, step_dt);
  time += step_dt;

  // Scheduled tasks
  while( !task_queue.empty() && task_queue.top().first <= time )
  {
    // the task may push other tasks
    // popping after processing may pop one of these tasks
    SmartPtr<TaskPhysics> task = task_queue.top().second;
    task_queue.pop();
    task->process(this);
  }
}


void Physics::scheduleTask(TaskPhysics *task, btScalar time)
{
  task_queue.push( TaskQueueValue(time, SmartPtr<TaskPhysics>(task)) );
}

void Physics::worldTickCallback(btDynamicsWorld *world, btScalar step)
{
  Physics *physics = (Physics*)world->getWorldUserInfo();

  std::set< SmartPtr<Object> >::const_iterator it_obj;
  for( it_obj = physics->tick_objs.begin(); it_obj != physics->tick_objs.end(); ++it_obj )
    (*it_obj)->tickCallback();
}


btRigidBody Physics::static_body( btRigidBody::btRigidBodyConstructionInfo(0,NULL,NULL) );



TaskBasic::TaskBasic(btScalar period):
  period(period), callback(NULL), cancelled(false)
{
}

void TaskBasic::process(Physics *ph)
{
  if( cancelled )
    return;
  if( callback == NULL )
    throw(Error("TaskBasic::process(): no callback"));

  callback(ph);
  if( period > 0.0 )
    ph->scheduleTask(this, ph->getTime() + period);
}

TaskLua::TaskLua(lua_State *L, int ref, btScalar period):
  period(period), cancelled(false), L(L), ref_obj(ref)
{
  if( this->ref_obj == LUA_NOREF || this->ref_obj == LUA_REFNIL )
    throw(LuaError(L, "TaskLua: invalid object reference"));
}

TaskLua::~TaskLua()
{
  luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);
}

void TaskLua::process(Physics *ph)
{
  if( cancelled )
    return;
  // Get the callback
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "callback");
  if( lua_isfunction(L, -1) )
    do_process_function(L);
  else if( lua_isthread(L, -1) )
    do_process_thread(L);
  else
    throw(Error("TaskLua: invalid callback"));

  if( period > 0 && ! cancelled )
    ph->scheduleTask(this, ph->getTime() + period);
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
    cancelled = true; // coroutine returned
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
    if( Physics::physics == NULL )
      Physics::physics = new Physics();
    store_ptr(L, Physics::physics);
    return 0;
  }

  LUA_DEFINE_SET0(init, init);
  LUA_DEFINE_GET(is_initialized, isInitialized);
  LUA_DEFINE_GET(get_time, getTime);
  LUA_DEFINE_SET0(pause, pause);
  LUA_DEFINE_SET0(unpause, unpause);
  LUA_DEFINE_SET0(toggle_pause, togglePause);


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(init);
    LUA_CLASS_MEMBER(is_initialized);
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
    btScalar period = 0.0;
    if( !lua_isnoneornil(L, 2) )
      period = LARG_f(2);

    data_ptr *ud = new_ptr(L);
    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    *ud = new TaskLua(L, ref, period);
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
  ref_children.reserve(n);
  for( int i=0; i<n; i++ )
  {
    btCollisionShape *sh = getChildShape(i);
    SmartPtr_add_ref(sh);
    ref_children.push_back(sh);
  }
}

void CompoundShapeSmart::clearChildReferences()
{
  std::vector<btCollisionShape *>::iterator it;
  for( it=ref_children.begin(); it!=ref_children.end(); ++it )
    SmartPtr_release( (*it) );
  ref_children.clear();
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
    lua_getfield(L, LUA_REGISTRYINDEX, name);
    lua_setmetatable(L, -2);

    *ud = p;
    // Increase ref count and store the pointer
    SmartPtr_add_ref(p);
  }

  static data_ptr get_ptr(lua_State *L)
  {
    return *(data_ptr *)luaL_checkudata(L, 1, name);
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

