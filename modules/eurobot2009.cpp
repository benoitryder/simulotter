#include "modules/eurobot2009.h"
#include "physics.h"
#include "display.h"
#include "graphics.h"
#include "log.h"

namespace eurobot2009 {


static const btScalar TABLE_HALF_X = 1.50_m;
static const btScalar TABLE_HALF_Y = 1.05_m;
static const btScalar WALL_HALF_WIDTH = 0.011_m;


SmartPtr<btCylinderShapeZ> OColElem::shape_(new btCylinderShapeZ(btVector3(0.035_m, 0.035_m, 0.015_m)));

OColElem::OColElem()
{
  setShape(shape_);
  setMass(0.100);
}

SmartPtr<btBoxShape> OLintel::shape_(new btBoxShape(btVector3(0.100_m, 0.035_m, 0.015_m)));
OLintel::OLintel()
{
  setShape(shape_);
  setMass(0.300);
}


const btScalar ODispenser::RADIUS = 0.040_m;
const btScalar ODispenser::HEIGHT = 0.150_m;
SmartPtr<btCylinderShapeZ> ODispenser::shape_(new btCylinderShapeZ(btVector3(RADIUS,RADIUS,HEIGHT/2)));

ODispenser::ODispenser()
{
  setShape(shape_);
  setColor(Color4::plexi);
  m_checkCollideWith = true;
}

void ODispenser::setPos(const btVector3& v, int side)
{
  btVector3 offset(0, 0, HEIGHT/2);
  switch(side) {
    case 0: offset.setY(-RADIUS); break;
    case 1: offset.setX(-RADIUS); break;
    case 2: offset.setY( RADIUS); break;
    case 3: offset.setX( RADIUS); break;
    default:
      throw(Error("invalid value for dispenser side"));
  }
  Object::setPos(v + offset);
}

void ODispenser::fill(Object* o, btScalar z)
{
  btVector3 pos = getPos();
  pos.setZ(z);
  o->setPos(pos);
}

void ODispenser::drawLast(Display*) const
{
  glColor4fv(color_);
  glPushMatrix();
  drawTransform(m_worldTransform);
  glTranslatef(0, 0, -HEIGHT/2);
  graphics::drawCylinder(RADIUS, HEIGHT, Display::draw_div);
  glPopMatrix();
}

bool ODispenser::checkCollideWithOverride(btCollisionObject* co)
{
  btRigidBody* o = btRigidBody::upcast(co);
  if(o) {
    if(btVector2(this->getPos()-o->getCenterOfMassPosition()).length() <= RADIUS) {
      return false;
    }
  }

  return btRigidBody::checkCollideWithOverride(co);
}


SmartPtr<btCompoundShape> OLintelStorage::shape_;
btBoxShape OLintelStorage::arm_shape_( btVector3(WALL_HALF_WIDTH,0.035_m,WALL_HALF_WIDTH) );
btBoxShape OLintelStorage::back_shape_( btVector3(0.100_m,WALL_HALF_WIDTH,0.030_m) );
btBoxShape OLintelStorage::bottom_shape_( btVector3(0.100_m,WALL_HALF_WIDTH,0.035_m) );

OLintelStorage::OLintelStorage()
{
  // First instance: initialize shape
  if(!shape_) {
    shape_ = new btCompoundShape();
    btTransform tr = btTransform::getIdentity();
    // Bottom
    tr.setOrigin( btVector3(0, 3*WALL_HALF_WIDTH-0.035_m, WALL_HALF_WIDTH-0.035_m) );
    shape_->addChildShape(tr, &bottom_shape_);
    // Back
    tr.setOrigin( btVector3(0, 0.035_m+WALL_HALF_WIDTH, 0.030_m-WALL_HALF_WIDTH) );
    shape_->addChildShape(tr, &back_shape_);
    // Left arm
    tr.setOrigin( btVector3(0.100_m-WALL_HALF_WIDTH, 0, 0) );
    shape_->addChildShape(tr, &arm_shape_);
    // Right arm
    tr.setOrigin( btVector3( -(0.100_m-WALL_HALF_WIDTH), 0, 0) );
    shape_->addChildShape(tr, &arm_shape_);
  }

  setShape(shape_);
  setColor(Color4::black);
}

void OLintelStorage::setPos(btScalar d, int side)
{
  btScalar x, y;
  switch(side) {
    case 0: x = d; y =  TABLE_HALF_Y+0.035_m; break;
    case 1: y = d; x =  TABLE_HALF_X+0.035_m; break;
    case 2: x = d; y = -TABLE_HALF_Y-0.035_m; break;
    case 3: y = d; x = -TABLE_HALF_X-0.035_m; break;
    default:
      throw(Error("invalid value for lintel storage side"));
  }
  OSimple::setPos( btVector3(x, y, 0.070_m+WALL_HALF_WIDTH) );
}

void OLintelStorage::fill(OLintel* o)
{
  btVector3 pos = getPos();
  pos[2] += 0.015_m+WALL_HALF_WIDTH+Physics::margin_epsilon;
  o->setPos(pos);
}



const btScalar Galipeur2009::Pachev::WIDTH = 0.080_m;
const btScalar Galipeur2009::Pachev::HEIGHT = 0.140_m;
const btScalar Galipeur2009::Pachev::Z_MAX = 0.080_m;
btScalar Galipeur2009::pachev_eject_speed = 0.001_m;


btBoxShape Galipeur2009::Pachev::shape_( 0.5*btVector3(WIDTH,WIDTH,HEIGHT) );

Galipeur2009::Galipeur2009(btScalar m): Galipeur(m)
{
  // Pàchev
  pachev_ = new Pachev(this);

  btTransform tr_a, tr_b;
  tr_a.setIdentity();
  tr_b.setIdentity();
  tr_a.getBasis().setEulerZYX(0, -M_PI_2, 0);
  tr_b.getBasis().setEulerZYX(0, -M_PI_2, 0);
  tr_a.setOrigin( btVector3(-D_SIDE, 0, -HEIGHT/2) );
  tr_b.setOrigin( btVector3(0.04_m, 0, -Pachev::HEIGHT/2) );
  pachev_link_ = new btSliderConstraint(*body_, *pachev_, tr_a, tr_b, true);
  pachev_link_->setLowerAngLimit(0);
  pachev_link_->setUpperAngLimit(0);

  pachev_link_->setPoweredLinMotor(true);
  pachev_link_->setTargetLinMotorVelocity(0);
  // always move at full speed: do not limit acceleration
  pachev_link_->setMaxLinMotorForce(100.0_m);
  pachev_link_->setLowerLinLimit(0);
  pachev_link_->setUpperLinLimit(0);

  pachev_state_ = PACHEV_RELEASE;
  pachev_moving_ = false;
}

Galipeur2009::~Galipeur2009()
{
  delete pachev_;
  delete pachev_link_;
}

Galipeur2009::Pachev::Pachev(Galipeur2009* robot):
  btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
{
  btVector3 inertia;
  shape_.calculateLocalInertia(0.01, inertia);
  setupRigidBody( btRigidBodyConstructionInfo(0.01,NULL,&shape_,inertia) );
  robot_ = robot;
  resetTrans();
}

Galipeur2009::Pachev::~Pachev()
{
}

bool Galipeur2009::Pachev::checkCollideWithOverride(btCollisionObject* co)
{
  if(co == robot_->body_) {
    return false;
  }
  btRigidBody* o = btRigidBody::upcast(co);
  if(o) {
    switch(robot_->pachev_state_) {
      case PACHEV_RELEASE: {
        if(btVector2(getCenterOfMassPosition() -
                     o->getCenterOfMassPosition()).length() <= WIDTH/2) {
          return false;
        }
      } break;

      case PACHEV_GRAB: {
        btVector3 diff = getCenterOfMassPosition() -
            o->getCenterOfMassPosition();
        if(btVector2(diff).length() <= WIDTH/2 && diff.z() < HEIGHT/2) {
          // Check if object is already constrained
          for(int i=0; i < getNumConstraintRefs(); i++) {
            if(&getConstraintRef(i)->getRigidBodyB() == o) {
              return false;
            }
          }

          // Move object on pàchev axis
          btTransform tr = o->getCenterOfMassTransform().inverseTimes(getCenterOfMassTransform());
          tr.setOrigin( btVector3(0,0,tr.getOrigin().getZ()) );

          btGeneric6DofConstraint* constraint = new btGeneric6DofConstraint(
              *this, *o, btTransform::getIdentity(), tr, false);
          for(int i=0; i<6; i++) {
            constraint->setLimit(i, 0, 0);
          }
          robot_->physics_->getWorld()->addConstraint(constraint, true);
          return false;
        }
      } break;

      case PACHEV_EJECT: {
        // Push colliding objects outside
        if(!co->isStaticOrKinematicObject()) {
          if(btRigidBody::checkCollideWithOverride(co)) {
            btVector2 v = robot_->pachev_link_->getFrameOffsetB()
                * getCenterOfMassPosition();
            v.normalize();
            o->translate( pachev_eject_speed * v );
          }
          return false;
        }
      } break;
    }
  }

  return btRigidBody::checkCollideWithOverride(co);
}

void Galipeur2009::addToWorld(Physics* physics)
{
  physics->getWorld()->addRigidBody(pachev_);
  physics->getWorld()->addConstraint(pachev_link_, true);
  Galipeur::addToWorld(physics);
}

void Galipeur2009::removeFromWorld()
{
  Physics* ph_bak = physics_;
  releaseObjects();
  Galipeur::removeFromWorld();
  ph_bak->getWorld()->removeConstraint(pachev_link_);
  ph_bak->getWorld()->removeRigidBody(pachev_);
}

void Galipeur2009::draw(Display* d) const
{
  Galipeur::draw(d);

  glPushMatrix();
  drawTransform(pachev_->getCenterOfMassTransform());
  btglScale(Pachev::WIDTH, Pachev::WIDTH, Pachev::HEIGHT);
  glutWireCube(1.0);
  glPopMatrix();
}


void Galipeur2009::setTrans(const btTransform& tr)
{
  body_->setCenterOfMassTransform(tr);
  pachev_->resetTrans();
}

void Galipeur2009::Pachev::resetTrans()
{
  btTransform tr;
  tr.setIdentity();
  tr.setOrigin( btVector3(-Galipeur2009::D_SIDE-WIDTH/2, 0, HEIGHT/2) );
  setCenterOfMassTransform(robot_->body_->getCenterOfMassTransform()*tr);
}

void Galipeur2009::releaseObjects()
{
  for(int i=pachev_->getNumConstraintRefs()-1; i>=0; i--) {
    btTypedConstraint* constraint = pachev_->getConstraintRef(i);
    if(constraint != pachev_link_) {
      physics_->getWorld()->removeConstraint(constraint);
      delete constraint;
    }
  }
}


void Galipeur2009::asserv()
{
  Galipeur::asserv();

  if(pachev_moving_) {
    if(btFabs(get_pachev_pos()-target_pachev_pos_) < threshold_pachev_) {
      pachev_link_->setTargetLinMotorVelocity(0);
      pachev_link_->setLowerLinLimit( target_pachev_pos_ );
      pachev_link_->setUpperLinLimit( target_pachev_pos_ );

      pachev_moving_ = false;
    }
  }
}

void Galipeur2009::order_pachev_move(btScalar h)
{
  target_pachev_pos_ = CLAMP(h,0,Galipeur2009::Pachev::Z_MAX);

  pachev_link_->setLowerLinLimit( 1 );
  pachev_link_->setUpperLinLimit( 0 );
  pachev_link_->setTargetLinMotorVelocity(
      (h > get_pachev_pos()) ? pachev_v_ : -pachev_v_
      );

  pachev_moving_ = true;
}

void Galipeur2009::order_pachev_release()
{
  releaseObjects();
  pachev_state_ = PACHEV_RELEASE;
}

void Galipeur2009::order_pachev_grab()
{
  pachev_state_ = PACHEV_GRAB;
}

void Galipeur2009::order_pachev_eject()
{
  if(pachev_state_ != PACHEV_RELEASE) {
    order_pachev_release();
  }

  pachev_state_ = PACHEV_EJECT;
}


}

