#include "galipeur.h"
#include "display.h"
#include "physics.h"
#include "sensors.h"
#include "log.h"


const btScalar Galipeur::Z_MASS = btScale(0.08);
const btScalar Galipeur::GROUND_CLEARANCE = btScale(0.009);
const btScalar Galipeur::ANGLE_OFFSET = -M_PI/2;

const btScalar Galipeur::HEIGHT  = btScale(0.300);
const btScalar Galipeur::SIDE    = btScale(0.110);
const btScalar Galipeur::W_BLOCK = btScale(0.030);
const btScalar Galipeur::R_WHEEL = btScale(0.0246);
const btScalar Galipeur::H_WHEEL = btScale(0.0127);

const btScalar Galipeur::D_SIDE  = ( SIDE + 2*W_BLOCK ) / btSqrt(3);
const btScalar Galipeur::D_WHEEL = ( W_BLOCK + 2*SIDE ) / btSqrt(3);
const btScalar Galipeur::A_SIDE  = 2*btAtan2( SIDE,    D_SIDE  );
const btScalar Galipeur::A_WHEEL = 2*btAtan2( W_BLOCK, D_WHEEL );
const btScalar Galipeur::RADIUS  = btSqrt(SIDE*SIDE+D_SIDE*D_SIDE);

SmartPtr<btCompoundShape> Galipeur::shape_;
btConvexHullShape Galipeur::body_shape_;
btBoxShape Galipeur::wheel_shape_( btVector3(H_WHEEL/2,R_WHEEL,R_WHEEL) );


Galipeur::Galipeur(btScalar m):
    color_(Color4(0.3))
{
  // First instance: initialize shape
  if( shape_ == NULL )
  {
    const btVector3 up(0,0,1);
    shape_ = new btCompoundShape();
    // Triangular body
    if( body_shape_.getNumPoints() == 0 )
    {
      btVector2 p = btVector2(RADIUS,0).rotated(-A_WHEEL/2);
      for( int i=0; i<3; i++ )
      {
        body_shape_.addPoint( btVector3(p.x(),p.y(),+HEIGHT/2) );
        body_shape_.addPoint( btVector3(p.x(),p.y(),-HEIGHT/2) );
        p.rotate(A_WHEEL);
        body_shape_.addPoint( btVector3(p.x(),p.y(),+HEIGHT/2) );
        body_shape_.addPoint( btVector3(p.x(),p.y(),-HEIGHT/2) );
        p.rotate(A_SIDE);
      }
    }
    shape_->addChildShape( btTransform( btQuaternion(up, -ANGLE_OFFSET),
                           btVector3(0,0,GROUND_CLEARANCE-(Z_MASS-HEIGHT/2))),
                          &body_shape_);

    // Wheels (use boxes instead of cylinders)
    btTransform tr = btTransform::getIdentity();
    tr.setRotation( btQuaternion(up, ANGLE_OFFSET) );
    btVector2 vw( D_WHEEL+H_WHEEL/2, 0 );
    vw.rotate(-ANGLE_OFFSET);
    tr.setOrigin( btVector3(vw.x(), vw.y(), -(Z_MASS - R_WHEEL)) );
    shape_->addChildShape(tr, &wheel_shape_);

    vw.rotate(2*M_PI/3);
    tr.setOrigin( btVector3(vw.x(), vw.y(), -(Z_MASS - R_WHEEL)) );
    tr.setRotation( btQuaternion(up, ANGLE_OFFSET+2*M_PI/3) );
    shape_->addChildShape(tr, &wheel_shape_);

    vw.rotate(-4*M_PI/3);
    tr.setOrigin( btVector3(vw.x(), vw.y(), -(Z_MASS - R_WHEEL)) );
    tr.setRotation( btQuaternion(up, ANGLE_OFFSET-2*M_PI/3) );
    shape_->addChildShape(tr, &wheel_shape_);
  }

  btVector3 inertia;
  shape_->calculateLocalInertia(m, inertia);
  body_ = new btRigidBody(
      btRigidBody::btRigidBodyConstructionInfo(m,NULL,shape_,inertia)
      );
  SmartPtr_add_ref(shape_);

  order_stop();
}

Galipeur::~Galipeur()
{
}

void Galipeur::addToWorld(Physics *physics)
{
  physics->getWorld()->addRigidBody(body_);
  Robot::addToWorld(physics);
}

void Galipeur::removeFromWorld()
{
  Physics *ph_bak = physics_;
  Robot::removeFromWorld();
  ph_bak->getWorld()->removeRigidBody(body_);
}

void Galipeur::draw(Display *d)
{
  glColor4fv(color_);

  glPushMatrix();
  drawTransform(body_->getCenterOfMassTransform());
  btglRotate(-ANGLE_OFFSET*180.0f/M_PI, 0.0f, 0.0f, 1.0f);

  if( d->callOrCreateDisplayList(this) ) {
    glPushMatrix();

    btglTranslate(0, 0, -Z_MASS);

    // Faces

    glPushMatrix();

    btglTranslate(0, 0, GROUND_CLEARANCE);

    btglScale(RADIUS, RADIUS, HEIGHT);
    btVector2 v;


    glBegin(GL_QUADS);

    v = btVector2(1,0).rotated(-A_WHEEL/2);
    btVector2 n(1,0); // normal vector
    for( int i=0; i<3; i++ )
    {
      // wheel side
      btglNormal3(n.x(), n.y(), 0.0);
      n.rotate(M_PI/3);

      btglVertex3(v.x(), v.y(), 0.0);
      btglVertex3(v.x(), v.y(), 1.0);
      v.rotate(A_WHEEL);
      btglVertex3(v.x(), v.y(), 1.0);
      btglVertex3(v.x(), v.y(), 0.0);

      // triangle side
      btglNormal3(n.x(), n.y(), 0.0);
      n.rotate(M_PI/3);

      btglVertex3(v.x(), v.y(), 0.0);
      btglVertex3(v.x(), v.y(), 1.0);
      v.rotate(A_SIDE);
      btglVertex3(v.x(), v.y(), 1.0);
      btglVertex3(v.x(), v.y(), 0.0);
    }

    glEnd();

    // Bottom
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, -1.0);
    v = btVector2(1,0).rotated(-A_WHEEL/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x(), v.y(), 0.0);
      v.rotate(A_WHEEL);
      btglVertex3(v.x(), v.y(), 0.0);
      v.rotate(A_SIDE);
    }
    glEnd();

    // Top
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, 1.0);
    v = btVector2(1,0).rotated(-A_WHEEL/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x(), v.y(), 1.0);
      v.rotate(A_WHEEL);
      btglVertex3(v.x(), v.y(), 1.0);
      v.rotate(A_SIDE);
    }
    glEnd();

    glPopMatrix();

    // Wheels (box shapes, but drawn using cylinders)
    btglTranslate(0, 0, R_WHEEL);
    btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
    btVector2 vw( D_WHEEL, 0 );

    glPushMatrix();
    btglTranslate(0, vw.y(), vw.x());
    glutSolidCylinder(R_WHEEL, H_WHEEL, Display::draw_div, Display::draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(2*M_PI/3);
    btglTranslate(0, vw.y(), vw.x());
    btglRotate(-120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(R_WHEEL, H_WHEEL, Display::draw_div, Display::draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(-4*M_PI/3);
    btglTranslate(0, vw.y(), vw.x());
    btglRotate(120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(R_WHEEL, H_WHEEL, Display::draw_div, Display::draw_div);
    glPopMatrix();

    glPopMatrix();

    d->endDisplayList();
  }

  glPopMatrix();
}


void Galipeur::setPosAbove(const btVector2 &pos)
{
  setPos( btVector3(pos.x(), pos.y(), Z_MASS + Physics::margin_epsilon) );
}


void Galipeur::asserv()
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  const btScalar tnow = physics_->getTime();
  const btScalar dt = tnow - ramp_last_t_;
  if( dt == 0 ) {
    return; // ramp start, wait for the next step
  }
  ramp_last_t_ = dt;

  // position
  if( ! order_xy_done() ) {
    btVector2 dxy = (*ckpt_) - btVector2(getPos());
    if( !lastCheckpoint() && dxy.length() < threshold_steering_ ) {
      ++ckpt_; // checkpoint change
      dxy = (*ckpt_) - btVector2(getPos());
    }
    if( lastCheckpoint() ) {
      ramp_xy_.var_dec = va_stop_;
      ramp_xy_.var_v0 = v_stop_;
    } else {
      ramp_xy_.var_dec = va_steering_;
      ramp_xy_.var_v0 = v_steering_;
    }
    set_v( dxy.normalized() * ramp_xy_.step(dt, dxy.length()) );
  }

  // angle
  if( ! order_a_done() ) {
    const btScalar da = btNormalizeAngle( target_a_-getAngle() );
    const btScalar new_av = ramp_a_.step(dt, btFabs(da));
    set_av( da > 0 ? new_av : -new_av );
  }
}


void Galipeur::set_v(btVector2 vxy)
{
  body_->activate();
  body_->setLinearVelocity( btVector3(vxy.x(), vxy.y(),
        body_->getLinearVelocity().z()) );
}

inline void Galipeur::set_av(btScalar v)
{
  body_->activate();
  btVector3 v3 = body_->getAngularVelocity();
  v3.setZ(v);
  body_->setAngularVelocity(v3);
}


void Galipeur::order_xy(btVector2 xy, bool rel)
{
  CheckPoints v(1);
  if( rel )
    xy += btVector2(getPos());
  v[0] = xy;
  order_trajectory(v);
}

void Galipeur::order_a(btScalar a, bool rel)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  if( rel )
    a += getAngle();
  target_a_ = btNormalizeAngle(a);

  ramp_last_t_ = physics_->getTime();
  ramp_a_.reset(getAngularVelocity());
}

void Galipeur::order_xya(btVector2 xy, btScalar a, bool rel)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  order_xy(xy, rel);
  order_a(a, rel);
}

void Galipeur::order_stop()
{
  checkpoints_.clear();
  ckpt_ = checkpoints_.end();
}


void Galipeur::order_trajectory(const std::vector<btVector2> &pts)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));
  if( pts.empty() )
    throw(Error("empty checkpoint list"));
  checkpoints_ = pts;
  ckpt_ = checkpoints_.begin();

  ramp_last_t_ = physics_->getTime();
  ramp_xy_.reset(getVelocity().length());
}


btScalar Galipeur::Quadramp::step(btScalar dt, btScalar d)
{
  btFullAssert( d >= 0 );
  const btScalar d_dec = 0.5 * (cur_v_*cur_v_ - var_v0*var_v0) / var_dec;
  if( d < d_dec ) {
    // deceleration
    cur_v_ = btMax(var_v0, cur_v_ - var_dec * dt);
  } else if( cur_v_ < var_v ) {
    // acceleration
    cur_v_ = btMin(var_v, cur_v_ + var_dec * dt);
  } else {
    // stable, nothing to do
  }
  return cur_v_;
}


