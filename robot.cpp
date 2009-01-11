#include <math.h>
#include <GL/freeglut.h>

#include "global.h"
#include "object.h"
#include "maths.h"


Robot::Robot()
{
  this->ref_obj = LUA_NOREF;
  this->ref_update = LUA_NOREF;
  this->ref_asserv = LUA_NOREF;
  this->ref_strategy = LUA_NOREF;
  this->L_strategy = NULL;

  this->team = TEAM_INVALID;
}

Robot::~Robot()
{
  lua_State *L = lm->get_L();
  if( ref_obj != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);
  if( ref_update != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_update);
  if( ref_asserv != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_asserv);
  if( ref_strategy != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_strategy);
}

void Robot::matchRegister(unsigned int team)
{
  if( getTeam() != TEAM_INVALID )
    throw(Error("robot is already registered"));

  if( match == NULL )
    throw(Error("no match to register the robot in"));

  this->team = match->registerRobot(this, team);
}


void Robot::draw()
{
  glColor4fv(match->getColor(team));
  Object::draw();
  drawDirection();
}

void Robot::drawDirection()
{
  glPushMatrix();
  drawTransform(m_worldTransform);

  btVector3 aabb_min, aabb_max;
  getAabb(aabb_min, aabb_max);

  btglTranslate(0, 0, aabb_max.getZ()-getPos().getZ()+cfg->draw_direction_r+cfg->draw_epsilon);
  btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(cfg->draw_direction_r, cfg->draw_direction_h, cfg->draw_div, cfg->draw_div);

  glPopMatrix();
}

void Robot::matchInit()
{
  if( ref_obj == LUA_NOREF )
    return;

  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_getfield(L, -1, "update");
  if( !lua_isnil(L, -1) )
    ref_update = luaL_ref(L, LUA_REGISTRYINDEX);
  else
    lua_pop(L, 1);
  lua_getfield(L, -1, "asserv");
  if( !lua_isnil(L, -1) )
    ref_asserv = luaL_ref(L, LUA_REGISTRYINDEX);
  else
    lua_pop(L, 1);
  lua_getfield(L, -1, "strategy");
  if( !lua_isnil(L, -1) )
  {
    ref_strategy = luaL_ref(L, LUA_REGISTRYINDEX);
    L_strategy = lua_newthread(L);
  }
  else
    lua_pop(L, 1);
}

void Robot::update()
{
  if( ref_update == LUA_NOREF )
    return this->do_update();

  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_update);
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  LuaManager::pcall(L, 1, 0);
}

void Robot::asserv()
{
  if( ref_asserv == LUA_NOREF )
    return this->do_asserv();

  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_asserv);
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  LuaManager::pcall(L, 1, 0);
}

void Robot::strategy()
{
  static int val = 0;
  if( ref_strategy == LUA_NOREF )
  {
    if( val != -1 )
      val = this->do_strategy(val);
    return;
  }

  if( L_strategy == NULL )
    return;

  int top = lua_gettop(L_strategy);
  lua_rawgeti(L_strategy, LUA_REGISTRYINDEX, ref_strategy);
  lua_rawgeti(L_strategy, LUA_REGISTRYINDEX, ref_obj);
  int ret = lua_resume(L_strategy, 1);
  if( ret == 0 )
  {
    lua_close(L_strategy);
    L_strategy = NULL;
  }
  else if( ret == LUA_YIELD )
    lua_settop(L_strategy, top);// Pop yield values
  else
    throw(LuaError(L_strategy));
}


RBasic::RBasic(const btVector3 &halfExtents, btScalar m)
{
  shape = new btBoxShape( halfExtents );
  setShape(shape);
  setMass(m);

  this->order = ORDER_NONE;
}

RBasic::~RBasic()
{
  delete shape;
}


void RBasic::do_asserv()
{
  // Go back: order which have priority
  if( order & ORDER_GO_BACK )
  {
    if( distance2(xy, target_back_xy) < threshold_xy )
    {
      set_v(0);
      order &= ~ORDER_GO_BACK;
    }
    else
    {
      set_v(-v_max);
      return;
    }
  }

  // Go in position
  if( order & ORDER_GO_XY )
  {
    if( distance2(xy, target_xy) < threshold_xy )
    {
      set_v(0);
      order &= ~ORDER_GO_XY;
    }
    else
    {
      // Aim target point, then move
      btScalar da = normA( (target_xy-xy).angle() - a );
      if( btFabs( da ) < threshold_a )
      {
        set_av(0);
        set_v(v_max);
      }
      else
      {
        set_av( btFsel(da, av_max, -av_max) );
        set_v(0);
      }
      return;
    }
  }

  // Turn
  if( order & ORDER_GO_A )
  {
    btScalar da = normA( target_a-a );
    if( btFabs( da ) < threshold_a )
    {
      set_av(0);
      order &= ~ORDER_GO_A;
    }
    else
    {
      set_av( btFsel(da, av_max, -av_max) );
      return;
    }
  }
}


void RBasic::order_xy(btVector2 xy, bool rel)
{
  LOG->trace("XY  %c %f,%f", rel?'+':'=', xy.x, xy.y);
  target_xy = xy;
  if( rel )
    target_xy += this->xy;

  order |= ORDER_GO_XY;
}

void RBasic::order_a(btScalar a, bool rel)
{
  LOG->trace("A  %c %f", rel?'+':'=', a);
  target_a = a;
  if( rel )
    target_a += this->a;
  target_a = normA(target_a);

  order |= ORDER_GO_A;
}

void RBasic::order_back(btScalar d)
{
  LOG->trace("BACK  %f", d);
  target_back_xy = xy - d*btVector2(1,0).rotate(a);

  order |= ORDER_GO_BACK;
}


void RBasic::do_update()
{
  //XXX robot may not be exactly aligned with Z axis
  xy = this->getPos();
  const btQuaternion &rot = this->getRot();
  a = rot.getAngle();
  if( rot.getZ() < 0 )
    a = -a;

  v = btVector2(this->getLinearVelocity()).length();
  av = this->getAngularVelocity().getZ();
  //XXX reset body to Z=0 (?)
}


class LuaRobot: public LuaClass<Robot>
{
  static int _ctor(lua_State *L)
  {
    Robot **ud = new_userdata(L);
    *ud = new Robot();

    lua_pushvalue(L, 1);
    (*ud)->ref_obj = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
  }

  static int get_team(lua_State *L)
  {
    unsigned int team = get_ptr(L)->getTeam();
    if( team == TEAM_INVALID )
      lua_pushnil(L);
    else
      push(L, team);
    return 1;
  }

  static int match_register(lua_State *L)
  {
    // Default team
    if( lua_isnone(L, 2) )
      get_ptr(L)->matchRegister();
    else
      get_ptr(L)->matchRegister(LARG_i(2));

    return 0;
  }

public:
  LuaRobot()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(get_team);
    LUA_REGFUNC(match_register);
  }

};


class LuaRBasic: public LuaClass<RBasic>
{
  static int _ctor(lua_State *L)
  {
    RBasic **ud = new_userdata(L);
    LOG->trace("LuaRBasic: BEGIN [%p]", ud);

    *ud = new RBasic( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)), LARG_f(5));
    lua_pushvalue(L, 1);
    (*ud)->ref_obj = luaL_ref(L, LUA_REGISTRYINDEX);

    LOG->trace("LuaRBasic: END");
    return 0;
  }

  LUA_DEFINE_GETN_SCALED(2, get_xy, get_xy)
  LUA_DEFINE_GET_SCALED(get_v, get_v)
  LUA_DEFINE_GET(get_a , get_a)
  LUA_DEFINE_GET(get_av, get_av)

  LUA_DEFINE_SET1(set_v_max,        set_v_max,        LARG_scaled)
  LUA_DEFINE_SET1(set_av_max,       set_av_max,       LARG_f)
  LUA_DEFINE_SET1(set_threshold_xy, set_threshold_xy, LARG_scaled)
  LUA_DEFINE_SET1(set_threshold_a,  set_threshold_a,  LARG_f)

  static int order_xy(lua_State *L)
  {
    get_ptr(L)->order_xy( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_bn(4) );
    return 0;
  }
  static int order_xya(lua_State *L)
  {
    get_ptr(L)->order_xya( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_f(4), LARG_bn(5) );
    return 0;
  }
  LUA_DEFINE_SET2(order_a,    order_a,    LARG_f, LARG_bn)
  LUA_DEFINE_SET1(order_back, order_back, LARG_scaled)
  LUA_DEFINE_SET0(order_stop, order_stop)

  LUA_DEFINE_GET(is_waiting, is_waiting)

public:
  LuaRBasic()
  {
    LUA_REGFUNC(_ctor);

    LUA_REGFUNC(get_xy);
    LUA_REGFUNC(get_v);
    LUA_REGFUNC(get_a);
    LUA_REGFUNC(get_av);

    LUA_REGFUNC(set_v_max);
    LUA_REGFUNC(set_av_max);
    LUA_REGFUNC(set_threshold_xy);
    LUA_REGFUNC(set_threshold_a);

    LUA_REGFUNC(order_xy);
    LUA_REGFUNC(order_xya);
    LUA_REGFUNC(order_a);
    LUA_REGFUNC(order_back);
    LUA_REGFUNC(order_stop);

    LUA_REGFUNC(is_waiting);
  }
};


LUA_REGISTER_SUB_CLASS(Robot,Object);
LUA_REGISTER_SUB_CLASS(RBasic,Robot);

