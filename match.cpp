#include "global.h"
#include "match.h"


Match::Match(const Color4 colors[], unsigned int team_nb, int duration)
{
  if( match != NULL )
    throw(Error("cannot create a second match"));
  match = this;

  this->ref_obj = LUA_NOREF;
  this->duration = duration;

  if( team_nb < 1 )
    throw(Error("invalid team number: %d", team_nb));

  this->team_nb = team_nb;
  this->colors = new Color4[team_nb];
  for( unsigned int i=0; i<team_nb; i++ )
    this->colors[i] = colors[i];
}

Match::~Match()
{
  lua_State *L = lm->get_L();
  if( ref_obj != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);

  delete[] colors;
}


unsigned int Match::registerRobot(Robot *r, unsigned int team)
{
  // Valid team: force
  if( team >= 0 && team < getTeamNb() )
  {
    if( robots.find(team) != robots.end() )
      throw(Error("team slot already in use: %d", team));
    robots[team] = r;
    return team;
  }
  else
  {
    for( unsigned int i=0; i<getTeamNb(); i++ )
      if( robots.find(i) == robots.end() )
      {
        robots[i] = r;
        return i;
      }
    throw(Error("no free team slot"));
  }
}


void Match::init(int fconf)
{
  if( ref_obj != LUA_NOREF )
  {
    lua_State *L = lm->get_L();
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
    lua_getfield(L, -1, "init");
    if( !lua_isnil(L, -1) )
    {
      lua_pushvalue(L, -2); // push self argument
      lua_pushinteger(L, fconf);
      LuaManager::pcall(L, 2, 0);
      lua_pop(L, 2);
      return;
    }
    else
      lua_pop(L, 2);
  }

  // No Lua function
  do_init(fconf);
}


class LuaMatch: public LuaClass<Match>
{
  static int _ctor(lua_State *L)
  {
    LOG->trace("LuaMatch: BEGIN");

    // Check color table
    luaL_checktype(L, 2, LUA_TTABLE);
    int nb = lua_objlen(L, 2);
    LOG->trace("  color nb: %d", nb);
    Color4 *colors = new Color4[nb];

    for( int i=0; i<nb; i++ )
    {
      lua_rawgeti(L, 2, i+1);
      if( lua_isnone(L, -1) ) // stop at the first missing integer index
      {
        lua_pop(L, 1);
        nb = i+1; // update team number
        break;
      }
      if( LuaManager::tocolor(L, -1, colors[i]) != 0 )
      {
        lua_pop(L, 1);
        delete[] colors;
        luaL_argerror(L, 2, "invalid type in table, color expected");
      }
      LOG->trace("  color %d : %f %f %f %f", i,
          colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
      lua_pop(L, 1);
    }

    Match **ud = new_userdata(L);
    if( lua_isnone(L, 3) )
      *ud = new Match(colors, nb);
    else if( lua_isnumber(L, 3) )
      *ud = new Match(colors, nb, lua_tointeger(L, 3));
    else
    {
      delete[] colors;
      luaL_argerror(L, 3, "number expected");
    }

    delete[] colors;

    lua_pushvalue(L, 1);
    (*ud)->ref_obj = luaL_ref(L, LUA_REGISTRYINDEX);

    LOG->trace("LuaMatch: END");
    return 0;
  }

  LUA_DEFINE_GET(get_team_nb, getTeamNb)

  static int get_color(lua_State *L)
  {
    push(L, get_ptr(L)->getColor(LARG_i(2)));
    return 1;
  }

public:
  LuaMatch()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(get_team_nb);
    LUA_REGFUNC(get_color);
  }

};

LUA_REGISTER_BASE_CLASS(Match);

