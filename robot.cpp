#include <math.h>
#include <ode/ode.h>

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

void Robot::init()
{
  Object::init();
  set_category(CAT_ROBOT);
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

void Robot::match_register(unsigned int team)
{
  if( get_team() != TEAM_INVALID )
    throw(Error("robot is already registered"));

  if( match == NULL )
    throw(Error("no match to register the robot in"));

  this->team = match->register_robot(this, team);
}


void Robot::draw()
{
  glColor4fv(match->get_color(team));
  Object::draw();
  draw_direction();
}

void Robot::draw_direction()
{
  dReal a[6];

  glPushMatrix();

  draw_move();
  get_aabb(a);
  const dReal *pos = get_pos();
  glTranslatef(0, 0, a[5]-pos[2]+cfg->draw_direction_r+cfg->draw_epsilon);
  glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(cfg->draw_direction_r, cfg->draw_direction_h, cfg->draw_div, cfg->draw_div);

  glPopMatrix();
}

void Robot::match_init()
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
  static int val = -1;
  if( ref_strategy == LUA_NOREF )
  {
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


RBasic::RBasic() {}

RBasic::RBasic(dReal lx, dReal ly, dReal lz, dReal m)
{
  add_geom( dCreateBox(0, lx, ly, lz) );
  set_mass(m);
}

void RBasic::init()
{
  Robot::init();

  this->order = ORDER_NONE;

  this->j2D = dJointCreatePlane2D(physics->get_world(), 0);
  this->jLMotor = dJointCreateLMotor(physics->get_world(), 0);
  dJointAttach(this->j2D, this->body, 0);
  dJointAttach(this->jLMotor, this->body, 0);
  dJointSetLMotorNumAxes(this->jLMotor, 1);
  // Axis is given in global coordinates
  // Assumes that robot has not been rotated yet
  dJointSetLMotorAxis(this->jLMotor, 0, 1, 1.0,0.0,0.0);
}

RBasic::~RBasic()
{
  dJointDestroy(j2D);
  dJointDestroy(jLMotor);
}


void RBasic::do_asserv()
{
  // Go back: priority order
  if( order & ORDER_GO_BACK )
  {
    if( dist2d(x, y, target_back_x, target_back_y) < threshold_xy )
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
    if( dist2d(x, y, target_x, target_y) < threshold_xy )
    {
      set_v(0);
      order &= ~ORDER_GO_XY;
    }
    else
    {
      // Aim target point, then move
      dReal da;
      if( target_x == x )
        da = ( target_y > y ) ? M_PI_2 : -M_PI_2;
      else
        da = atan2(target_y-y, (target_x==x)?0.0001:target_x-x);
      da = norma(da-a);
      if( absf( da ) < threshold_a )
      {
        set_av(0);
        set_v(v_max);
      }
      else
      {
        set_av(av_max*signf(da));
        set_v(0);
      }
      return;
    }
  }

  // Turn
  if( order & ORDER_GO_A )
  {
    dReal da = norma(target_a-a);
    if( absf( da ) < threshold_a )
    {
      set_av(0);
      order &= ~ORDER_GO_A;
    }
    else
    {
      set_av(av_max*signf(da));
      return;
    }
  }
}


void RBasic::order_xy(dReal x, dReal y, bool rel)
{
  LOG->trace("XY  %c %f,%f", rel?'+':'=', x, y);
  target_x = x;
  target_y = y;
  if( rel )
  {
    target_x += this->x;
    target_y += this->y;
  }

  order |= ORDER_GO_XY;
}

void RBasic::order_a(dReal a, bool rel)
{
  LOG->trace("A  %c %f", rel?'+':'=', a);
  target_a = a;
  if( rel )
    target_a += this->a;
  a = norma(a);

  order |= ORDER_GO_A;
}

void RBasic::order_back(dReal d)
{
  LOG->trace("BACK  %f", d);
  target_back_x = this->x - d*cos(this->a);
  target_back_y = this->y - d*sin(this->a);

  order |= ORDER_GO_BACK;
}


void RBasic::set_dv_max(dReal dv)
{
  dMass mass;
  dBodyGetMass(body, &mass);
  dJointSetLMotorParam(jLMotor, dParamFMax, dv*mass.mass);
}

void RBasic::set_dav_max(dReal dav)
{
  dMass mass;
  dBodyGetMass(body, &mass);
  dJointSetPlane2DAngleParam(j2D, dParamFMax, dav/mass.I[11]);
}


void RBasic::do_update()
{
  const dReal *pos = dBodyGetPosition(body);
  const dReal *vel = dBodyGetLinearVel(body);
  const dReal *quat_ptr = dBodyGetQuaternion(body);

  // Reset body to Z=0 (cf. ODE manual)
  dReal quat[4];
  quat[0] = quat_ptr[0];
  quat[1] = 0;
  quat[2] = 0; 
  quat[3] = quat_ptr[3]; 
  dReal quat_len = sqrt(quat[0]*quat[0] + quat[3]*quat[3]);
  quat[0] /= quat_len;
  quat[3] /= quat_len;
  dBodySetQuaternion(body, quat);

  av = dBodyGetAngularVel(body)[2];
  dBodySetAngularVel(body, 0, 0, av);
  av *= cfg->step_dt;

  x = pos[0];
  y = pos[1];
  a = norma(2*acos(quat[0])) * signf(quat[3]);
  v = sqrt(vel[0]*vel[0] + vel[1]*vel[1]);
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

  LUA_DEFINE_GET(get_team)

  static int match_register(lua_State *L)
  {
    // Default team
    if( lua_isnone(L, 2) )
      get_ptr(L)->match_register();
    else
      get_ptr(L)->match_register(LARG_i(2));

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

    // Empty constructor
    if( lua_isnone(L, 2) )
    {
      LOG->trace("  proto: empty");
      *ud = new RBasic();
    }
    // Box constructor
    else
    {
      LOG->trace("  proto: box");
      *ud = new RBasic(LARG_f(2), LARG_f(3), LARG_f(4), LARG_f(5));
    }

    lua_pushvalue(L, 1);
    (*ud)->ref_obj = luaL_ref(L, LUA_REGISTRYINDEX);

    LOG->trace("LuaRBasic: END");
    return 0;
  }

  LUA_DEFINE_GET(get_x )
  LUA_DEFINE_GET(get_y )
  LUA_DEFINE_GET(get_a )
  LUA_DEFINE_GET(get_v )
  LUA_DEFINE_GET(get_av)

  LUA_DEFINE_SET1(set_dv_max,       LARG_f)
  LUA_DEFINE_SET1(set_dav_max,      LARG_f)
  LUA_DEFINE_SET1(set_v_max,        LARG_f)
  LUA_DEFINE_SET1(set_av_max,       LARG_f)
  LUA_DEFINE_SET1(set_threshold_xy, LARG_f)
  LUA_DEFINE_SET1(set_threshold_a,  LARG_f)

  LUA_DEFINE_SET3(order_xy,   LARG_f, LARG_f, LARG_bn)
  LUA_DEFINE_SET2(order_a,    LARG_f, LARG_bn)
  LUA_DEFINE_SET4(order_xya,  LARG_f, LARG_f, LARG_f, LARG_bn)
  LUA_DEFINE_SET1(order_back, LARG_f)
  LUA_DEFINE_SET0(order_stop)

  LUA_DEFINE_GET(is_waiting)

public:
  LuaRBasic()
  {
    LUA_REGFUNC(_ctor);

    LUA_REGFUNC(get_x);
    LUA_REGFUNC(get_y);
    LUA_REGFUNC(get_a);
    LUA_REGFUNC(get_v);
    LUA_REGFUNC(get_av);

    LUA_REGFUNC(set_dv_max);
    LUA_REGFUNC(set_dav_max);
    LUA_REGFUNC(set_v_max);
    LUA_REGFUNC(set_av_max);
    LUA_REGFUNC(set_threshold_xy);
    LUA_REGFUNC(set_threshold_a);

    LUA_REGFUNC(order_xy);
    LUA_REGFUNC(order_a);
    LUA_REGFUNC(order_xya);
    LUA_REGFUNC(order_back);
    LUA_REGFUNC(order_stop);

    LUA_REGFUNC(is_waiting);
  }
};


LUA_REGISTER_SUB_CLASS(Robot,Object);
LUA_REGISTER_SUB_CLASS(RBasic,Robot);

