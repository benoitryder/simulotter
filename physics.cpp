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

  // Parralel solver pour du multithread (?)
  solver = new btSequentialImpulseConstraintSolver();

  world = new btDiscreteDynamicsWorld(
      dispatcher, broadphase, solver, col_config
      );

  //Note: gravity must be set again after loading lua script
  world->setGravity(btVector3(0,0,cfg->gravity_z));

  pause_state = false;
}

Physics::~Physics()
{
  std::vector<Object*>::iterator it;
  for( it = objs.begin(); it != objs.end(); ++it )
    delete *it;
  objs.clear();

  delete world;
  delete solver;
  delete dispatcher;
  delete col_config;
  delete broadphase;
}


void Physics::step()
{
  if( this->pause_state )
    return;

  //TODO timeStep / subSteps / fixedTimeStep / ...
  world->stepSimulation(cfg->step_dt, 1, cfg->step_dt);

  // Update robot values, do asserv and strategy
  std::map<unsigned int,Robot*> &robots = match->getRobots();
  std::map<unsigned int,Robot*>::iterator itr;
  for( itr=robots.begin(); itr!=robots.end(); ++itr )
  {
    (*itr).second->update();
    (*itr).second->asserv();
    (*itr).second->strategy();
  }
}


void Physics::addObject(Object *obj)
{
  if( !obj->isInitialized() )
    throw Error("Physics::addObject(): object is not initialized");
  world->addRigidBody(obj);
  objs.push_back(obj);
}

btRigidBody Physics::static_body( btRigidBody::btRigidBodyConstructionInfo(0,NULL,NULL) );



/** @name Lua collision shapes
 *
 * Shapes is not a usual class: instances are created using a specific method
 * for each shape.
 * Instances are not tables with an \e _ud fields be userdata.
 * new_userdata() and get_ptr() are redefined for this purpose.
 *
 * @todo Garbage collection, object deletion, ...
 * @todo Compound shapes
 */
//@{

class LuaShape: public LuaClass<btCollisionShape>
{
  static btCollisionShape **new_userdata(lua_State *L)
  {
    // check that we use Class:method and not Class.method
    luaL_checktype(L, 1, LUA_TUSERDATA);
    btCollisionShape **ud = (btCollisionShape **)lua_newuserdata(L, sizeof(btCollisionShape *));
    lua_getfield(L, LUA_REGISTRYINDEX, name);
    lua_setmetatable(L, -2);
    return ud;
  }

  static btCollisionShape *get_ptr(lua_State *L)
  {
    return *(btCollisionShape **)luaL_checkudata(L, 1, name);
  }

  static int _ctor(lua_State *L)
  {
    return luaL_error(L, "no Shape constructor, use creation methods");
  }

  static int sphere(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btSphereShape(LARG_scaled(2));
    return 1;
  }

  static int box(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btBoxShape( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
    return 1;
  }

  static int capsuleX(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCapsuleShapeX(LARG_scaled(2), LARG_scaled(3));
    return 1;
  }
  static int capsuleY(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCapsuleShape(LARG_scaled(2), LARG_scaled(3));
    return 1;
  }
  static int capsuleZ(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCapsuleShapeZ(LARG_scaled(2), LARG_scaled(3));
    return 1;
  }

  static int cylinderX(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCylinderShapeX( btVector3(LARG_scaled(3), LARG_scaled(2), LARG_scaled(2)) );
    return 1;
  }
  static int cylinderY(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCylinderShape( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(2)) );
    return 1;
  }
  static int cylinderZ(lua_State *L)
  {
    btCollisionShape **ud = new_userdata(L);
    *ud = new btCylinderShapeZ( btVector3(LARG_scaled(2), LARG_scaled(2), LARG_scaled(3)) );
    return 1;
  }

  // Garbage collector
  static int gc(lua_State *L)
  {
    /*TODO
    btCollisionShape *shape = get_ptr(L);
    // Don't destroy used geoms (should not happen, just in case)
    if( dGeomGetSpace(geom) == 0 )
      dGeomDestroy( geom );
      */
    return 0;
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
    functions.push_back((LuaRegFunc){"__gc", gc});
  }
};

template<> const char *LuaClass<btCollisionShape>::name = "Shape";
static LuaShape register_LuaShape;
template<> const char *LuaClass<btCollisionShape>::base_name = NULL;

//@}
