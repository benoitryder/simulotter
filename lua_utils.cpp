#include "global.h"
#include "lua_utils.h"


/// Convenient macro for registering global objects
#define LUA_REG_GLOBAL(name,f) lua_register(L, #name, f)

/// Convenient macro for adding a function in an object at the top of the top stack
#define LUA_REG_FIELD(name,f) (lua_pushcfunction(L,f),lua_setfield(L, -2, #name))


LuaManager::LuaManager()
{
  this->L = luaL_newstate();
  if( this->L == NULL )
    throw(LuaError("state creation failed (memory allocation error)"));

  // Load some standard libraries
	lua_pushcfunction(L, luaopen_base);   lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_math);   lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_table);  lua_call(L, 0, 0);

  // Register global functions
  LUA_REG_GLOBAL(trace, lua_trace);

  // Init Lua stuff of other classes
  cfg->lua_init(L);
  LuaClassBase::init(L);
}

LuaManager::~LuaManager()
{
  lua_close(L);
}


void LuaManager::do_file(const char *filename)
{
  int ret;

  ret = luaL_loadfile(L, filename);
  if( ret != 0 )
    throw(LuaError(L, ret));

  ret = lua_pcall((lua_State *)L, 0, LUA_MULTRET, 0);
  if( ret != 0 )
    throw(LuaError(L, ret));
}

void LuaManager::checkcolor(lua_State *L, int narg, Color4 c)
{
  luaL_checktype(L, narg, LUA_TTABLE);

  int n = lua_objlen(L, narg);
  luaL_argcheck(L, n==4 || n==3, narg, // alpha is optional
      "invalid component count, 3 or 4 expected");

  lua_pushnil(L);
  int i;
  float f;
  for( i=0; i<4 && lua_next(L, narg); i++ )
  {
    luaL_argcheck(L, lua_isnumber(L, -1), narg,
        "invalid color component, number expected");
    f = lua_tonumber(L, -1);
    luaL_argcheck(L, f>=0.0 && f<=1.0, narg,
        "invalid color component value");
    c[i] = f;
    lua_pop(L, 1);
  }
}


int LuaManager::lua_trace(lua_State *L)
{
  LOG->trace("== %s", luaL_checkstring(L,1) );
  return 0;
}


std::vector<LuaClassBase*> LuaClassBase::classes;
const char *LuaClassBase::registry_class_mt_name = "simulotter_class";


int LuaClassBase::new_class(lua_State *L)
{
  int n = lua_gettop(L);

  LOG->trace("LUA class constructor");

  // Check base class
  if( lua_isuserdata(L, 1) )
    luaL_checkudata(L, 1, registry_class_mt_name);
  else if( !lua_isnoneornil(L, 1) )
    luaL_checktype(L, 1, LUA_TTABLE);

  lua_newtable(L);
  int new_class = lua_gettop(L);

  LOG->trace("  set the base class");
  // Set the base class
  if( lua_isnoneornil(L, 1) )
    lua_pushnil(L);
  else
    lua_pushvalue(L, 1);
  lua_setfield(L, new_class, "_base");

  // Is there a constructor?
  if( n > 1 )
  {
    LOG->trace("  set constructor");
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    lua_setfield(L, new_class, "_ctor");
  }

  lua_pushvalue(L, new_class);
  lua_setfield(L, new_class, "__index");

  LOG->trace("  set metatable");
  lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
  lua_setmetatable(L, -2);

  LOG->trace("  DONE");

  return 1;
}


int LuaClassBase::new_instance(lua_State *L)
{
  int n = lua_gettop(L);

  LOG->trace("instance new");

  lua_newtable(L);

  // Set class table as object metatable
  lua_pushvalue(L, 1);
  if( lua_isuserdata(L, 1) )
  {
    LOG->trace("  userdata class");
    LuaClassBase *ud = *(LuaClassBase **)luaL_checkudata(L, 1, registry_class_mt_name);
    lua_getfield(L, LUA_REGISTRYINDEX, ud->get_name());
    lua_remove(L, -2);
  }
  lua_setmetatable(L, -2);

  // Is there a constructor?
  lua_getfield(L, 1, "_ctor");
  if( lua_isfunction(L, -1) )
  {
    // Copy arguments
    lua_pushvalue(L, -2);
    for( int i=2; i<=n; i++ )
      lua_pushvalue(L, i);
    LOG->trace("  call _ctor");
    LuaManager::pcall(L, n, 0);
  }
  else
    lua_pop(L, 1); // Pop the constructor

  return 1;
}

int LuaClassBase::index_class(lua_State *L)
{
  LOG->trace("class/mt __index [ %s ]", lua_tostring(L, 2));

  // Predefined class: get field from method table
  if( lua_isuserdata(L, 1) )
  {
    LuaClassBase *ud = *(LuaClassBase **)luaL_checkudata(L, 1, registry_class_mt_name);
    LOG->trace("  userdata class: %s", ud->get_name());
    lua_getfield(L, LUA_REGISTRYINDEX, ud->get_name());
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);
    lua_remove(L, -2);
    LOG->trace("  DONE: class mt/__index");
    return 1;
  }

  lua_pushstring(L, "_base");
  lua_rawget(L, 1);

  // No base class, return nil (which is already on the stack)
  if( lua_isnil(L, -1) )
  {
    LOG->trace("  no base classe");
    LOG->trace("  DONE: class mt/__index");
    return 1;
  }

  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  lua_remove(L, -2);// Pop _base/_ud

  LOG->trace("  DONE: class mt/__index");
  return 1;
}


void LuaClassBase::init(lua_State *L)
{
  // Construct the class metatable
  luaL_newmetatable(L, registry_class_mt_name);
  lua_pushcfunction(L, new_instance);
  lua_setfield(L, -2, "__call");
  lua_pushcfunction(L, index_class);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  LUA_REG_GLOBAL(class, new_class);

  // Add subclasses
  std::vector<LuaClassBase*>::iterator it;
  for( it=classes.begin(); it!=classes.end(); it++ )
    (*it)->create(L);

  // Now, all classes are registered, set _base field
  for( it=classes.begin(); it!=classes.end(); it++ )
  {
    const char *base = (*it)->get_base_name();
    if( base == NULL )
      continue;
    lua_getfield(L, LUA_REGISTRYINDEX, (*it)->get_name());
    lua_getfield(L, LUA_GLOBALSINDEX, base);
    lua_setfield(L, -2, "_base");
    lua_pop(L, 1);
  }
}

