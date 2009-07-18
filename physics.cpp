#include <string.h>
#include "global.h"
#include "robot.h"
#include "maths.h"


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

  pause_state = false;
  step_dt = 0;
  time = 0;
}

Physics::~Physics()
{
  objs.clear();
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
    LOG->trace("physics already initialized, init() call skipped");
    return;
  }
  if( cfg->step_dt <= 0 )
    throw(Error("invalid step_dt value for Physics initialization"));
  step_dt = cfg->step_dt;
  world->setGravity(btVector3(0,0,cfg->gravity_z));
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
    task_queue.top().second->process(this);
    task_queue.pop();
  }

  // Update robot values, do asserv and strategy
  std::map<unsigned int, SmartPtr<Robot> > &robots = match->getRobots();
  std::map<unsigned int, SmartPtr<Robot> >::iterator itr;
  for( itr=robots.begin(); itr!=robots.end(); ++itr )
  {
    (*itr).second->update();
    (*itr).second->asserv();
    (*itr).second->strategy();
  }
}


void Physics::scheduleTask(TaskPhysics *task, btScalar time)
{
  task_queue.push( TaskQueueValue(time, SmartPtr<TaskPhysics>(task)) );
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

TaskLua::TaskLua(int ref, btScalar period):
  period(period), cancelled(false), ref_obj(ref)
{
  if( this->ref_obj == LUA_NOREF || this->ref_obj == LUA_REFNIL )
    throw(LuaError("invalid object reference for TaskLua"));
}

TaskLua::~TaskLua()
{
  lua_State *L = lm->get_L();
  luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);
}

void TaskLua::process(Physics *ph)
{
  if( cancelled )
    return;
  // Get the callback
  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "callback");
  if( !lua_isfunction(L, -1) )
    throw(LuaError("TaskLua: invalid callback"));
  LuaManager::pcall(L, 0, 0);

  if( period > 0 )
    ph->scheduleTask(this, ph->getTime() + period);
}



/** @name Lua Physics class
 * The constructor is a factory and returns the singleton.
 */
class LuaPhysics: public LuaClass<Physics>
{
  static int _ctor(lua_State *L)
  {
    if( physics == NULL )
      physics = new Physics();
    store_ptr(L, physics);
    return 0;
  }

  LUA_DEFINE_SET0(init, init);
  LUA_DEFINE_GET(is_initialized, isInitialized);
  LUA_DEFINE_GET(get_time, getTime);
  LUA_DEFINE_SET0(pause, pause);
  LUA_DEFINE_SET0(unpause, unpause);
  LUA_DEFINE_SET0(toggle_pause, togglePause);

public:
  LuaPhysics()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(init);
    LUA_REGFUNC(is_initialized);
    LUA_REGFUNC(get_time);
    LUA_REGFUNC(pause);
    LUA_REGFUNC(unpause);
    LUA_REGFUNC(toggle_pause);
  }
};


LUA_REGISTER_BASE_CLASS(Physics);


/** @name Lua Task class
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
    *ud = new TaskLua(ref, period);
    SmartPtr_add_ref(*ud);
    return 0;
  }

  LUA_DEFINE_SET0(cancel, cancel);

  static int schedule(lua_State *L)
  {
    if( !physics )
      return luaL_error(L, "physics is not created, cannot schedule");
    SmartPtr<TaskLua> task = get_ptr(L,1);
    if( lua_isnoneornil(L, 2) )
      physics->scheduleTask(task);
    else
      physics->scheduleTask(task, LARG_f(2));
    return 0;
  }

public:
  LuaTask()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(cancel);
    LUA_REGFUNC(schedule);
  }
};

template<> const char *LuaClass<TaskLua>::name = "Task";
static LuaTask register_LuaTask;
template<> const char *LuaClass<TaskLua>::base_name = NULL;


/** @name Lua collision shapes
 *
 * Shapes is not a usual class: instances are created using a specific method
 * for each shape, there are no constructor.
 * Instances are not tables with an \e _ud fields but userdata.
 * store_ptr() and get_ptr() are redefined for this purpose.
 *
 * @todo Compound shapes
 */
//@{

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

public:
  LuaShape()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(sphere);
    LUA_REGFUNC(box);
    LUA_REGFUNC(capsuleX);
    LUA_REGFUNC(capsuleY);
    LUA_REGFUNC(capsuleZ);
    LUA_REGFUNC(cylinderX);
    LUA_REGFUNC(cylinderY);
    LUA_REGFUNC(cylinderZ);
  }
};

template<> const char *LuaClass<btCollisionShape>::name = "Shape";
static LuaShape register_LuaShape;
template<> const char *LuaClass<btCollisionShape>::base_name = NULL;

//@}
