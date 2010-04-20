#include <cmath>
#include <GL/freeglut.h>
#include "robot.h"
#include "physics.h"
#include "config.h"
#include "lua_utils.h"
#include "log.h"


Robot::Robot()
{
}

RBasic::RBasic()
{
  body_ = NULL;
  order_ = ORDER_NONE;
}

RBasic::RBasic(btCollisionShape *shape, btScalar m)
{
  body_ = NULL;
  order_ = ORDER_NONE;
  setup(shape, m);
}

void RBasic::setup(btCollisionShape *shape, btScalar m)
{
  if( body_ != NULL )
    throw(Error("robot already setup"));
  btVector3 inertia;
  shape->calculateLocalInertia(m, inertia);
  body_ = new btRigidBody(
      btRigidBody::btRigidBodyConstructionInfo(m,NULL,shape,inertia)
      );
  SmartPtr_add_ref(shape);
}

RBasic::~RBasic()
{
  if( body_ != NULL )
  {
    SmartPtr_release(body_->getCollisionShape());
    delete body_;
  }
}

void RBasic::addToWorld(Physics *physics)
{
  physics->getWorld()->addRigidBody(body_);
  Robot::addToWorld(physics);
}

void RBasic::removeFromWorld()
{
  Physics *ph_bak = physics_;
  Robot::removeFromWorld();
  ph_bak->getWorld()->removeRigidBody(body_);
}


void RBasic::draw()
{
  glColor4fv(color_);
  glPushMatrix();
  drawTransform(body_->getCenterOfMassTransform());
  drawShape(body_->getCollisionShape());
  drawDirection();
  glPopMatrix();
}

void RBasic::drawDirection()
{
  btVector3 aabb_min, aabb_max;
  body_->getAabb(aabb_min, aabb_max);

  btglTranslate(0, 0, aabb_max.getZ()-getPos().getZ()+cfg.draw_direction_r+cfg.draw_epsilon);
  btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(cfg.draw_direction_r, cfg.draw_direction_h, cfg.draw_div, cfg.draw_div);
}


void RBasic::asserv()
{
  // Go back: order which have priority
  if( order_ & ORDER_GO_BACK )
  {
    if( distance2(xy_, target_back_xy_) < threshold_xy_ )
    {
      set_v(0);
      order_ &= ~ORDER_GO_BACK;
    }
    else
    {
      set_v(-v_max_);
      return;
    }
  }

  // Go in position
  if( order_ & ORDER_GO_XY )
  {
    if( distance2(xy_, target_xy_) < threshold_xy_ )
    {
      set_v(0);
      order_ &= ~ORDER_GO_XY;
    }
    else
    {
      // Aim target point, then move
      btScalar da = btNormalizeAngle( (target_xy_-xy_).angle() - a_ );
      if( btFabs( da ) < threshold_a_ )
      {
        set_av(0);
        set_v(v_max_);
      }
      else
      {
        set_v(0);
        set_av( btFsel(da, av_max_, -av_max_) );
      }
      return;
    }
  }

  // Turn
  if( order_ & ORDER_GO_A )
  {
    btScalar da = btNormalizeAngle( target_a_-a_ );
    if( btFabs( da ) < threshold_a_ )
    {
      set_av(0);
      order_ &= ~ORDER_GO_A;
    }
    else
    {
      set_av( btFsel(da, av_max_, -av_max_) );
      return;
    }
  }
}


void RBasic::order_xy(btVector2 xy, bool rel)
{
  target_xy_ = xy;
  if( rel )
    target_xy_ += xy_;

  order_ |= ORDER_GO_XY;
}

void RBasic::order_a(btScalar a, bool rel)
{
  target_a_ = a;
  if( rel )
    target_a_ += a_;
  target_a_ = btNormalizeAngle(target_a_);

  order_ |= ORDER_GO_A;
}

void RBasic::order_back(btScalar d)
{
  target_back_xy_ = xy_ - d*btVector2(1,0).rotated(a_);

  order_ |= ORDER_GO_BACK;
}


void RBasic::update()
{
  xy_ = this->getPos();
  btScalar p, r;
  this->getRot().getEulerYPR(a_,p,r);

  v_ = btVector2(body_->getLinearVelocity()).length();
  av_ = body_->getAngularVelocity().getZ();
}


inline void RBasic::set_v(btScalar v)
{
  body_->activate();
  btVector2 vxy = btVector2(v,0).rotated(a_);
  body_->setLinearVelocity( btVector3(vxy.x, vxy.y,
        body_->getLinearVelocity().z()) );
}

inline void RBasic::set_av(btScalar v)
{
  body_->activate();
  body_->setAngularVelocity( btVector3(0,0,v) );
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

