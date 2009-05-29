#include <string.h>
#include "global.h"


Config::Config()
{
  gravity_z = scale(-CONST_EARTH_GRAVITY);
  step_dt = 0.001;
  time_scale = 1.0;
  match_fconf = -1;
  drop_epsilon = scale(0.001);

  draw_epsilon = scale(0.0001);
  draw_div = 20;
  draw_direction_r = scale(0.05);
  draw_direction_h = scale(0.10);

  perspective_fov = 45.0;
  perspective_near = scale(0.1);
  perspective_far = scale(300.0);

  screen_x = 800;
  screen_y = 600;
  fullscreen = false;
  fps = 60;
  camera_step_angle  = 0.1;
  camera_step_linear = scale(0.1);
  camera_mouse_coef  = 0.1;

  bg_color = Color4(0.8);

  log_flush = true;
}


const char *Config::registry_name = "Config";


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
  if( strcmp(name, #n) == 0 ) {LuaManager::push(L, config->n); return 1;}
#define CONFIG_INDEX_VAL_SCALED(n) \
  if( strcmp(name, #n) == 0 ) {LuaManager::push(L, unscale(config->n)); return 1;}

  CONFIG_INDEX_VAL_SCALED(gravity_z);
  CONFIG_INDEX_VAL(step_dt);
  CONFIG_INDEX_VAL(time_scale);
  CONFIG_INDEX_VAL(match_fconf);
  CONFIG_INDEX_VAL_SCALED(drop_epsilon);
  CONFIG_INDEX_VAL_SCALED(draw_epsilon);
  CONFIG_INDEX_VAL(draw_div);
  CONFIG_INDEX_VAL_SCALED(draw_direction_r);
  CONFIG_INDEX_VAL_SCALED(draw_direction_h);
  CONFIG_INDEX_VAL(perspective_fov);
  CONFIG_INDEX_VAL_SCALED(perspective_near);
  CONFIG_INDEX_VAL_SCALED(perspective_far);
  CONFIG_INDEX_VAL(screen_x);
  CONFIG_INDEX_VAL(screen_y);
  CONFIG_INDEX_VAL(fps);
  CONFIG_INDEX_VAL(fullscreen);
  CONFIG_INDEX_VAL(log_flush);

#undef CONFIG_INDEX_VAL
#undef CONFIG_INDEX_VAL_SCALED

  if( strcmp(name, "bg_color") == 0 )
  {
    LuaManager::push(L, config->bg_color);
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

  CONFIG_NEWINDEX_VAL(gravity_z,        LARG_scaled);
  CONFIG_NEWINDEX_VAL(step_dt,          LARG_f);
  CONFIG_NEWINDEX_VAL(time_scale,       LARG_f);
  CONFIG_NEWINDEX_VAL(match_fconf,      LARG_i);
  CONFIG_NEWINDEX_VAL(drop_epsilon,     LARG_scaled);
  CONFIG_NEWINDEX_VAL(draw_epsilon,     LARG_scaled);
  CONFIG_NEWINDEX_VAL(draw_div,         LARG_i);
  CONFIG_NEWINDEX_VAL(draw_direction_r, LARG_scaled);
  CONFIG_NEWINDEX_VAL(draw_direction_h, LARG_scaled);
  CONFIG_NEWINDEX_VAL(perspective_fov,  LARG_f);
  CONFIG_NEWINDEX_VAL(perspective_near, LARG_scaled);
  CONFIG_NEWINDEX_VAL(perspective_far,  LARG_scaled);
  CONFIG_NEWINDEX_VAL(screen_x,         LARG_i);
  CONFIG_NEWINDEX_VAL(screen_y,         LARG_i);
  CONFIG_NEWINDEX_VAL(fps,              LARG_f);
  CONFIG_NEWINDEX_VAL(fullscreen,       LARG_b);
  CONFIG_NEWINDEX_VAL(log_flush,        LARG_b);

#undef CONFIG_NEWINDEX_VAL

  if( strcmp(name, "bg_color") == 0 )
  {
    LuaManager::checkcolor(L, 3, config->bg_color);
    return 0;
  }

  return luaL_error(L, "invalid configuration value: %s", name);
}

