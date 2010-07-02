#include <cmath>
#include "robot.h"
#include "display.h"
#include "physics.h"
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

  btglTranslate(0, 0, aabb_max.getZ()-getPos().getZ()+DIRECTION_CONE_R+Display::DRAW_EPSILON);
  btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(DIRECTION_CONE_R, DIRECTION_CONE_H, Display::DRAW_DIV, Display::DRAW_DIV);
}

const float RBasic::DIRECTION_CONE_R = btScale(0.05);
const float RBasic::DIRECTION_CONE_H = btScale(0.10);


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


