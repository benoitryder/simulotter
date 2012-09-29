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


void RBasic::draw(Display *d) const
{
  glColor4fv(color_);
  glPushMatrix();
  drawTransform(body_->getCenterOfMassTransform());
  if( d->callOrCreateDisplayList(body_->getCollisionShape()) ) {
    drawShape(body_->getCollisionShape());
    d->endDisplayList();
  }
  drawDirection(d);
  glPopMatrix();
}

void RBasic::drawDirection(Display *) const
{
  btVector3 aabb_min, aabb_max;
  body_->getAabb(aabb_min, aabb_max);

  btglTranslate(0, 0, aabb_max.getZ()-getPos().getZ()+DIRECTION_CONE_R+Display::draw_epsilon);
  btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
  glutSolidCone(DIRECTION_CONE_R, DIRECTION_CONE_H, Display::draw_div, Display::draw_div);
}

const float RBasic::DIRECTION_CONE_R = 0.05_m;
const float RBasic::DIRECTION_CONE_H = 0.10_m;


void RBasic::asserv()
{
  // Go back: order which have priority
  if( order_ & ORDER_GO_BACK ) {
    const btVector2 xy = getPos();
    if( distance2(xy, target_back_xy_) < threshold_xy ) {
      set_v(0);
      order_ &= ~ORDER_GO_BACK;
    } else {
      set_v(-v_max);
      return;
    }
  }

  // Go in position
  if( order_ & ORDER_GO_XY ) {
    const btVector2 xy = getPos();
    if( distance2(xy, target_xy_) < threshold_xy ) {
      set_v(0);
      order_ &= ~ORDER_GO_XY;
    } else {
      // Aim target point, then move
      btScalar da = btNormalizeAngle( (target_xy_-xy).angle() - getAngle() );
      if( btFabs( da ) < threshold_a ) {
        set_av(0);
        set_v(v_max);
      } else {
        set_v(0);
        set_av( btFsel(da, av_max, -av_max) );
      }
      return;
    }
  }

  // Turn
  if( order_ & ORDER_GO_A ) {
    btScalar da = btNormalizeAngle( target_a_-getAngle() );
    if( btFabs( da ) < threshold_a ) { set_av(0);
      order_ &= ~ORDER_GO_A;
    } else {
      set_av( btFsel(da, av_max, -av_max) );
      return;
    }
  }
}


void RBasic::order_xy(btVector2 xy, bool rel)
{
  target_xy_ = xy;
  if( rel ) {
    target_xy_ += getPos();
  }

  order_ |= ORDER_GO_XY;
}

void RBasic::order_a(btScalar a, bool rel)
{
  target_a_ = a;
  if( rel )
    target_a_ += getAngle();
  target_a_ = btNormalizeAngle(target_a_);

  order_ |= ORDER_GO_A;
}

void RBasic::order_back(btScalar d)
{
  target_back_xy_ = btVector2(getPos()) - d*btVector2(1,0).rotated(getAngle());

  order_ |= ORDER_GO_BACK;
}


inline void RBasic::set_v(btScalar v)
{
  body_->activate();
  btVector2 vxy = btVector2(v,0).rotated(getAngle());
  body_->setLinearVelocity( btVector3(vxy.x(), vxy.y(),
        body_->getLinearVelocity().z()) );
}

inline void RBasic::set_av(btScalar v)
{
  body_->activate();
  body_->setAngularVelocity( btVector3(0,0,v) );
}


