#include <string.h>
#include "global.h"
#include "config.h"
#include "colors.h"


Config::Config()
{
  gravity_z = -CONST_EARTH_GRAVITY;
  step_dt = 0.001;
  contacts_nb = 5;
  drop_epsilon = 0.001;

  draw_epsilon = 0.0001;
  draw_div = 20;
  draw_direction_r = 0.05;
  draw_direction_h = 0.10;

  perspective_fov = 45.0;
  perspective_near = 0.1;
  perspective_far = 300.0;

  screen_x = 800;
  screen_y = 600;
  fullscreen = false;
  fps = 60;

  COLOR_COPY(bg_color, (Color4)COLOR_GRAY(0.8));

  log_flush = true;
}


const char *Config::registry_name = LUA_REGISTRY_PREFIX "config";


void Config::lua_init(lua_State *L)
{
  // Create user data
  Config **ud = (Config **)lua_newuserdata(L, sizeof(this));
  *ud = this;

  // Create the metatable
  luaL_newmetatable(L, registry_name);
  lua_pushcfunction(L, &lua_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, &lua_newindex);
  lua_setfield(L, -2, "__newindex");

  // Set the metatable and the global value
  lua_setmetatable(L, -2);
  lua_setfield(L, LUA_GLOBALSINDEX, "config");
}

int Config::lua_index(lua_State *L)
{
  Config *config = *(Config **)luaL_checkudata(L, 1, registry_name);
  const char *name = luaL_checkstring(L, 2);

#define CONFIG_INDEX_VAL(n) \
  if( strcmp(name, #n) == 0 ) {LuaClassBase::push(L, config->n); return 1;}

  CONFIG_INDEX_VAL(gravity_z);
  CONFIG_INDEX_VAL(step_dt);
  CONFIG_INDEX_VAL(contacts_nb);
  CONFIG_INDEX_VAL(drop_epsilon);
  CONFIG_INDEX_VAL(draw_epsilon);
  CONFIG_INDEX_VAL(draw_div);
  CONFIG_INDEX_VAL(draw_direction_r);
  CONFIG_INDEX_VAL(draw_direction_h);
  CONFIG_INDEX_VAL(perspective_fov);
  CONFIG_INDEX_VAL(perspective_near);
  CONFIG_INDEX_VAL(perspective_far);
  CONFIG_INDEX_VAL(screen_x);
  CONFIG_INDEX_VAL(screen_y);
  CONFIG_INDEX_VAL(fps);
  CONFIG_INDEX_VAL(fullscreen);
  CONFIG_INDEX_VAL(log_flush);

  if( strcmp(name, "bg_color") == 0 )
  {
    LuaClassBase::push(L, config->bg_color);
    return 1;
  }

  return luaL_error(L, "invalid configuration value: %s", name);
}

int Config::lua_newindex(lua_State *L)
{
  Config *config = *(Config **)luaL_checkudata(L, 1, registry_name);
  const char *name = luaL_checkstring(L, 2);

#define CONFIG_NEWINDEX_VAL(n,m) \
  if( strcmp(name, #n) == 0 ) {config->n = m(3); return 0;}

  CONFIG_NEWINDEX_VAL(gravity_z,        LARG_f);
  CONFIG_NEWINDEX_VAL(step_dt,          LARG_f);
  CONFIG_NEWINDEX_VAL(contacts_nb,      LARG_i);
  CONFIG_NEWINDEX_VAL(drop_epsilon,     LARG_f);
  CONFIG_NEWINDEX_VAL(draw_epsilon,     LARG_f);
  CONFIG_NEWINDEX_VAL(draw_div,         LARG_i);
  CONFIG_NEWINDEX_VAL(draw_direction_r, LARG_f);
  CONFIG_NEWINDEX_VAL(draw_direction_h, LARG_f);
  CONFIG_NEWINDEX_VAL(perspective_fov,  LARG_f);
  CONFIG_NEWINDEX_VAL(perspective_near, LARG_f);
  CONFIG_NEWINDEX_VAL(perspective_far,  LARG_f);
  CONFIG_NEWINDEX_VAL(screen_x,         LARG_i);
  CONFIG_NEWINDEX_VAL(screen_y,         LARG_i);
  CONFIG_NEWINDEX_VAL(fps,              LARG_f);
  CONFIG_NEWINDEX_VAL(fullscreen,       LARG_b);
  CONFIG_NEWINDEX_VAL(log_flush,        LARG_b);

  if( strcmp(name, "bg_color") == 0 )
  {
    LuaManager::checkcolor(L, 3, config->bg_color);
    return 0;
  }

  return luaL_error(L, "invalid configuration value: %s", name);
}
 
