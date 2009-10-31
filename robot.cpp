#include <math.h>
#include <GL/freeglut.h>

#include "global.h"


Robot::Robot()
{
}

RBasic::RBasic()
{
  this->body = NULL;
  this->order = ORDER_NONE;
}

RBasic::RBasic(btCollisionShape *shape, btScalar m)
{
  this->body = NULL;
  this->order = ORDER_NONE;
  setup(shape, m);
}

void RBasic::setup(btCollisionShape *shape, btScalar m)
{
  if( this->body != NULL )
    throw(Error("robot already setup"));
  btVector3 inertia;
  shape->calculateLocalInertia(m, inertia);
  this->body = new btRigidBody(
      btRigidBody::btRigidBodyConstructionInfo(m,NULL,shape,inertia)
      );
  SmartPtr_add_ref(shape);
}

RBasic::~RBasic()
{
  //TODO remove from the world
  if( body != NULL )
  {
    SmartPtr_release(body->getCollisionShape());
    delete body;
  }
}

void RBasic::addToWorld(Physics *physics)
{
  physics->getWorld()->addRigidBody(body);
  Robot::addToWorld(physics);
}

void RBasic::draw()
{
  glColor4fv(color);
  glPushMatrix();
  drawTransform(body->getCenterOfMassTransform());
  drawShape(body->getCollisionShape());
  drawDirection();
  glPopMatrix();
}

void RBasic::drawDirection()
{
  btVector3 aabb_min, aabb_max;
  body->getAabb(aabb_min, aabb_max);

  btglTranslate(0, 0, aabb_max.getZ()-getPos().getZ()+cfg->draw_direction_r+cfg->draw_epsilon);
  btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(cfg->draw_direction_r, cfg->draw_direction_h, cfg->draw_div, cfg->draw_div);
}


void RBasic::asserv()
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
        set_v(0);
        set_av( btFsel(da, av_max, -av_max) );
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
  target_xy = xy;
  if( rel )
    target_xy += this->xy;

  order |= ORDER_GO_XY;
}

void RBasic::order_a(btScalar a, bool rel)
{
  target_a = a;
  if( rel )
    target_a += this->a;
  target_a = normA(target_a);

  order |= ORDER_GO_A;
}

void RBasic::order_back(btScalar d)
{
  target_back_xy = xy - d*btVector2(1,0).rotated(a);

  order |= ORDER_GO_BACK;
}


void RBasic::update()
{
  xy = this->getPos();
  btScalar p, r;
  this->getRot().getEulerYPR(a,p,r);

  v = btVector2(body->getLinearVelocity()).length();
  av = body->getAngularVelocity().getZ();
}


inline void RBasic::set_v(btScalar v)
{
  //XXX we have to force activation, is it a Bullet bug?
  body->activate();
  btVector2 vxy = btVector2(v,0).rotated(a);
  body->setLinearVelocity( btVector3(vxy.x, vxy.y,
        body->getLinearVelocity().z()) );
}

inline void RBasic::set_av(btScalar v)
{
  //XXX we have to force activation, is it a Bullet bug?
  body->activate();
  body->setAngularVelocity( btVector3(0,0,v) );
}



class LuaRobot: public LuaClass<Robot>
{
  static int _ctor(lua_State *L)
  {
    return luaL_error(L, "Robot class is abstract, no constructor");
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
  }

};


class LuaRBasic: public LuaClass<RBasic>
{
  static int _ctor(lua_State *L)
  {
    btCollisionShape *shape;
    shape = *(btCollisionShape **)luaL_checkudata(L, 2, LUA_REGISTRY_PREFIX "Shape"); //XXX
    store_ptr(L, new RBasic( shape, LARG_f(3)));
    return 0;
  }

  static int set_color(lua_State *L)
  {
    Color4 color;
    LuaManager::checkcolor(L, 2, color);
    get_ptr(L,1)->setColor( color );
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
    get_ptr(L,1)->order_xy( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_bn(4) );
    return 0;
  }
  static int order_xya(lua_State *L)
  {
    get_ptr(L,1)->order_xya( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_f(4), LARG_bn(5) );
    return 0;
  }
  LUA_DEFINE_SET2(order_a,    order_a,    LARG_f, LARG_bn)
  LUA_DEFINE_SET1(order_back, order_back, LARG_scaled)
  LUA_DEFINE_SET0(order_stop, order_stop)

  LUA_DEFINE_SET0(update, update)
  LUA_DEFINE_SET0(asserv, asserv)
  LUA_DEFINE_GET(is_waiting, is_waiting)


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(set_color);

    LUA_CLASS_MEMBER(get_xy);
    LUA_CLASS_MEMBER(get_v);
    LUA_CLASS_MEMBER(get_a);
    LUA_CLASS_MEMBER(get_av);

    LUA_CLASS_MEMBER(set_v_max);
    LUA_CLASS_MEMBER(set_av_max);
    LUA_CLASS_MEMBER(set_threshold_xy);
    LUA_CLASS_MEMBER(set_threshold_a);

    LUA_CLASS_MEMBER(order_xy);
    LUA_CLASS_MEMBER(order_xya);
    LUA_CLASS_MEMBER(order_a);
    LUA_CLASS_MEMBER(order_back);
    LUA_CLASS_MEMBER(order_stop);

    LUA_CLASS_MEMBER(update);
    LUA_CLASS_MEMBER(asserv);
    LUA_CLASS_MEMBER(is_waiting);
  }
};


LUA_REGISTER_SUB_CLASS(Robot,Object);
LUA_REGISTER_SUB_CLASS(RBasic,Robot);

