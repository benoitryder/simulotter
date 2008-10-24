#include <math.h>
#include <ode/ode.h>

#include "global.h"
#include "object.h"
#include "maths.h"


Robot::Robot(dGeomID geom, dBodyID body):
  ObjectDynamic(geom, body)
{
  ctor_init();
}

Robot::Robot(dGeomID geom, dReal m):
  ObjectDynamic(geom, m)
{
  ctor_init();
}

void Robot::ctor_init()
{
  LOG->trace("Robot: NEW (init)");
  set_category(CAT_ROBOT);
  robots.push_back(this);
  this->ref_obj = LUA_NOREF;
  this->ref_update = LUA_NOREF;
  this->ref_asserv = LUA_NOREF;
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
}

void Robot::draw()
{
  glColor3fv(rules->get_color(team));
  draw_geom(geom);
  draw_direction();
}

void Robot::draw_direction()
{
  dReal a[6];

  glPushMatrix();

  draw_move();
  dGeomGetAABB(geom, a);
  glTranslatef(0, 0, (a[5]-a[4])/2+cfg->draw_direction_r+cfg->draw_epsilon);
  glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(cfg->draw_direction_r, cfg->draw_direction_h, cfg->draw_div, cfg->draw_div);

  glPopMatrix();
}

void Robot::init()
{
  if( ref_obj == LUA_NOREF )
    return;

  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_getfield(L, -1, "asserv");
  if( !lua_isnil(L, -1) )
    ref_update = luaL_ref(L, LUA_REGISTRYINDEX);
  else
    lua_pop(L, 1);
  lua_getfield(L, -1, "asserv");
  if( !lua_isnil(L, -1) )
    ref_asserv = luaL_ref(L, LUA_REGISTRYINDEX);
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
  if( ref_update == LUA_NOREF )
    return this->do_asserv();

  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_asserv);
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  LuaManager::pcall(L, 1, 0);
}

std::vector<Robot*> Robot::robots;


RBasic::RBasic(dGeomID geom, dBodyID body):
  Robot(geom, body)
{
  ctor_init();
}

RBasic::RBasic(dGeomID geom, dReal m):
  Robot(geom, m)
{
  ctor_init();
}

RBasic::RBasic(dReal lx, dReal ly, dReal lz, dReal m):
  Robot(dCreateBox(0, lx, ly, lz), m)
{
  ctor_init();
}

void RBasic::ctor_init()
{
  LOG->trace("Rbasic: NEW (init)");
  this->order = ORDER_NONE;

  this->j2D = dJointCreatePlane2D(physics->get_world(), 0);
  this->jLMotor = dJointCreateLMotor(physics->get_world(), 0);
  dJointAttach(this->j2D, this->body, 0);
  dJointAttach(this->jLMotor, this->body, 0);
  dJointSetLMotorNumAxes(this->jLMotor, 1);
  // Axis is given in global coordinates
  // It assumes that robot has not been rotated yet
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
      //XXX don't set motors if they are already set
      //TODO case 'x = target_x'
      dReal da = norma(atan2(y-target_y, x-target_x)-a);
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
  target_a = a;
  if( rel )
    target_a += this->a;
  a = norma(a);

  order |= ORDER_GO_A;
}

void RBasic::order_back(dReal d)
{
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
    Robot **ud = get_userdata(L);
    dGeomID geom = (dGeomID)LARG_lud(2);
    *ud = new Robot(geom, LARG_f(3));

    lua_pushvalue(L, 1);
    (*ud)->set_ref_obj(luaL_ref(L, LUA_REGISTRYINDEX));
    return 0;
  }

  LUA_DEFINE_GET(get_team)

public:
  LuaRobot()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(get_team);
  }

};


class LuaRBasic: public LuaClass<RBasic>
{
  static int _ctor(lua_State *L)
  {
    RBasic **ud = get_userdata(L);
    luaL_checkany(L, 2);
    LOG->trace("LuaRBasic: BEGIN [%p]", ud);

    if( lua_islightuserdata(L, 2) )
    {
      LOG->trace("  proto: geom");
      dGeomID geom = (dGeomID)LARG_lud(2);
      *ud = new RBasic(geom, LARG_f(3));
    }
    else if( lua_isnumber(L, 2) )
    {
      LOG->trace("  proto: box");
      *ud = new RBasic(LARG_f(2), LARG_f(3), LARG_f(4), LARG_f(5));
    }
    else
      luaL_argerror(L, 2, "expected geom or number");

    LOG->trace("  set object reference");
    lua_pushvalue(L, 1);
    (*ud)->set_ref_obj(luaL_ref(L, LUA_REGISTRYINDEX));

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
  }
};


LUA_REGISTER_SUB_CLASS(Robot,ObjectDynamic);
LUA_REGISTER_SUB_CLASS(RBasic,Robot);

