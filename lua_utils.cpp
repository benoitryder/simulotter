#include <time.h>
#include "global.h"


/// Convenient macro for adding a function in an object at the top of the top stack
#define LUA_REG_FIELD(name,f) (lua_pushcfunction(L,f),lua_setfield(L, -2, #name))


LuaMainModule LuaManager::main_module;

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
	lua_pushcfunction(L, luaopen_io);     lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_package);lua_call(L, 0, 0);

  // Initialize random seed
  lua_getglobal(L, "math");
  lua_getfield(L, -1, "randomseed");
  lua_pushinteger(L, ::time(NULL));
  lua_pcall(L, 1, 0, 0);

  // Register global functions
  lua_register(L, "trace", lua_trace);

  // Init Lua stuff of other classes
  cfg->lua_init(L);
  LuaClassBase::init(L);

  // Import main module
  main_module.import(L, NULL);
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

  ret = lua_pcall(L, 0, LUA_MULTRET, 0);
  if( ret != 0 )
    throw(LuaError(L, ret));
}


int LuaManager::tocolor(lua_State *L, int index, Color4 &c)
{
  // If index is relative to the top, get the absolute position
  if( index < 0 )
    index = lua_gettop(L) + 1 + index;

  if( !lua_istable(L, index) )
  {
    lua_pushstring(L, "table expected");
    return 1;
  }

  int n = lua_objlen(L, index);
  if( n != 4 && n != 3 ) // alpha is optional
  {
    lua_pushstring(L, "invalid component count, 3 or 4 expected");
    return 1;
  }

  int i;
  float f;
  for( i=0; i<n; i++ )
  {
    lua_rawgeti(L, index, i+1);
    if( lua_isnone(L, -1) )
    {
      lua_pop(L, 1);
      lua_pushstring(L, "color component is missing");
      return 1;
    }
    if( !lua_isnumber(L, -1) )
    {
      lua_pop(L, 1);
      lua_pushstring(L, "invalid color component, number expected");
      return 1;
    }
    f = lua_tonumber(L, -1);
    if( f < 0.0 || f > 1.0 )
    {
      lua_pop(L, 1);
      lua_pushstring(L, "invalid color component value");
      return 1;
    }
    c.rgba[i] = f;
    lua_pop(L, 1);
  }
  if( n < 4 )
    c.rgba[3] = 1.0f;

  return 0;
}

void LuaManager::checkcolor(lua_State *L, int narg, Color4 &c)
{
  int ret = tocolor(L, narg, c);
  if( ret != 0 )
    throw(LuaError(L, "invalid color"));
}

int LuaManager::totransform(lua_State *L, int index, btTransform &tr)
{
  // If index is relative to the top, get the absolute position
  if( index < 0 )
    index = lua_gettop(L) + 1 + index;

  if( !lua_istable(L, index) )
  {
    lua_pushstring(L, "table expected");
    return 1;
  }

  int n = lua_objlen(L, index);
  if( n != 3 && n != 6 ) // rotation is optional
  {
    lua_pushstring(L, "invalid element count, 3 or 6 expected");
    return 1;
  }

  int i;
  float f[6];
  for( i=0; i<n; i++ )
  {
    lua_rawgeti(L, index, i+1);
    if( lua_isnone(L, -1) )
    {
      lua_pop(L, 1);
      lua_pushstring(L, "element is missing");
      return 1;
    }
    if( !lua_isnumber(L, -1) )
    {
      lua_pop(L, 1);
      lua_pushstring(L, "invalid element, number expected");
      return 1;
    }
    f[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }

  tr.setIdentity();
  tr.setOrigin( scale(btVector3(f[0],f[1],f[2])) );
  if( n == 6 )
    tr.getBasis().setEulerZYX( f[3], f[4], f[5] );

  return 0;
}

void LuaManager::checktransform(lua_State *L, int narg, btTransform &tr)
{
  int ret = totransform(L, narg, tr);
  if( ret != 0 )
    throw(LuaError(L, "invalid transform"));
}


int LuaManager::lua_trace(lua_State *L)
{
  LOG->trace("== %s", luaL_checkstring(L,1) );
  return 0;
}


std::vector<LuaClassBase*> LuaClassBase::classes;
const char *LuaClassBase::registry_class_mt_name = LUA_REGISTRY_PREFIX "class";

LuaClassBase::LuaClassBase()
{
  // register the class
  classes.push_back(this);
}


void LuaClassBase::create(lua_State *L)
{
  if( luaL_newmetatable(L, get_name()) == 0 )
    throw(Error("class '%s' already created", get_name()));

  lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
  lua_setmetatable(L, -2);

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  // Add methods
  std::vector<LuaRegFunc>::iterator it;
  for( it=functions.begin(); it!=functions.end(); ++it )
  {
    lua_pushcfunction(L, (*it).f);
    lua_setfield(L, -2, (*it).name);
  }

  // Create the class object
  this->create_ud(L);
  lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
  lua_setmetatable(L, -2);

  // Set class object on the metable
  lua_setfield(L, -2, "_cls");

  lua_pop(L, 1); // Pop the metatable
}

void LuaClassBase::init_base_class(lua_State *L)
{
  if( get_base_name() == NULL )
    return;

  // get metatable
  lua_getfield(L, LUA_REGISTRYINDEX, get_name());

  lua_getfield(L, LUA_REGISTRYINDEX, get_base_name());
  if( lua_isnil(L, -1) )
    throw(Error("undefined base class '%s'", get_base_name()));
  lua_pushstring(L, "_cls");
  lua_rawget(L, -2);
  if( lua_isnil(L, -1) )
    throw(Error("class object not found for base class '%s'", get_base_name()));
  lua_setfield(L, -3, "_base");

  lua_pop(L, 2); // pop class and base class metatables
}


int LuaClassBase::new_class(lua_State *L)
{
  int n = lua_gettop(L);

  // Check base class
  if( lua_isuserdata(L, 1) )
    luaL_checkudata(L, 1, registry_class_mt_name);
  else if( !lua_isnoneornil(L, 1) )
    luaL_checktype(L, 1, LUA_TTABLE);

  lua_newtable(L);
  int new_class = lua_gettop(L);

  // Set the base class
  if( lua_isnoneornil(L, 1) )
    lua_pushnil(L);
  else
    lua_pushvalue(L, 1);
  lua_setfield(L, new_class, "_base");

  // Is there a constructor?
  if( n > 1 )
  {
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    lua_setfield(L, new_class, "_ctor");
  }

  lua_pushvalue(L, new_class);
  lua_setfield(L, new_class, "__index");

  lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
  lua_setmetatable(L, -2);

  return 1;
}


int LuaClassBase::new_instance(lua_State *L)
{
  int n = lua_gettop(L);

  lua_newtable(L);

  // Set class table as object metatable
  lua_pushvalue(L, 1);
  if( lua_isuserdata(L, 1) )
  {
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
    LuaManager::pcall(L, n, 0);
  }
  else
    lua_pop(L, 1); // Pop the constructor

  return 1;
}

int LuaClassBase::index_class(lua_State *L)
{
  // Predefined class: get field from method table
  if( lua_isuserdata(L, 1) )
  {
    LuaClassBase *ud = *(LuaClassBase **)luaL_checkudata(L, 1, registry_class_mt_name);
    lua_getfield(L, LUA_REGISTRYINDEX, ud->get_name());
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);
    lua_remove(L, -2);
    return 1;
  }

  lua_pushstring(L, "_base");
  lua_rawget(L, 1);

  // No base class, return nil (which is already on the stack)
  if( lua_isnil(L, -1) )
    return 1;

  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  lua_remove(L, -2);// Pop _base/_ud

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

  lua_register(L, "class", new_class);

  std::vector<LuaClassBase*>::iterator it;
  for( it=classes.begin(); it!=classes.end(); ++it )
    (*it)->create(L);
  for( it=classes.begin(); it!=classes.end(); ++it )
    (*it)->init_base_class(L);
}


void LuaModule::import(lua_State *L, const char *name)
{
  if( name == NULL )
    lua_pushvalue(L, LUA_GLOBALSINDEX);
  else
  {
    lua_getglobal(L, name);
    if( lua_isnil(L, -1) )
    {
      // new table
      lua_pop(L,1);
      lua_newtable(L);
      lua_pushvalue(L, -1);
      lua_setglobal(L, name);
    }
    else if( !lua_istable(L, -1) )
      throw(Error("cannot create LUA module: '%s' exists and is not a table", name));
  }

  this->do_import(L);

  lua_pop(L, 1);
}

void LuaModule::import_class(lua_State *L, const char *cls, const char *name)
{
  lua_getfield(L, LUA_REGISTRYINDEX, cls);
  if( lua_isnil(L, -1) )
    throw(Error("LUA class '%s' not found", cls));
  lua_pushstring(L, "_cls");
  lua_rawget(L, -2);
  if( lua_isnil(L, -1) )
    throw(Error("class object not found for class '%s'", cls));

  lua_setfield(L, -3, name);
  lua_pop(L, 1);
}


void LuaMainModule::do_import(lua_State *L)
{
  LUA_IMPORT_CLASS(Object);
  LUA_IMPORT_CLASS(OSimple);
  LUA_IMPORT_CLASS(OGround);
  LUA_IMPORT_CLASS(Robot);
  LUA_IMPORT_CLASS(RBasic);
  LUA_IMPORT_CLASS(Galipeur);
  LUA_IMPORT_CLASS(Physics);
  LUA_IMPORT_CLASS(Shape);
  LUA_IMPORT_CLASS(Task);
  LUA_IMPORT_CLASS(Display);
  LUA_IMPORT_CLASS(OSD);
}

