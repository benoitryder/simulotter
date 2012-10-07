#include <cassert>
#include "modules/eurobot2011.h"
#include "display.h"
#include "log.h"

namespace eurobot2011 {


const btVector2 OGround2011::SIZE = btVector2(3.0_m, 2.1_m);
const btScalar OGround2011::SQUARE_SIZE = 0.350_m;
const btScalar OGround2011::START_SIZE = 0.400_m;

OGround2011::OGround2011():
    OGroundSquareStart(SIZE,
                       Color4(0x24,0x91,0x40), // RAL 6024
                       Color4(0xc7,0x17,0x12), // RAL 3020
                       Color4(0x00,0x3b,0x80)) // RAL 5017
{
  setStartSize(START_SIZE);
}

void OGround2011::drawDisplayList() const
{
  OGroundSquareStart::drawDisplayList();

  btglTranslate(0, 0, size_[2]/2);

  // draw the checkerboard
  glPushMatrix();
  btglNormal3(0.0, 0.0, 1.0);

  btglTranslate(0, 0, Display::draw_epsilon);
  btglScale(SQUARE_SIZE, SQUARE_SIZE, 1);

  // 6 lines, 6 columns, 1 square out of 2 (for each color)

  glColor4fv(color_t1_);
  for(int i=-3; i<3; i++) {
    for(int j=-3; j<3; j++) {
      if((i+j)%2 == 0) {
        btglRect(i, j, i+1, j+1);
      }
    }
  }

  glColor4fv(color_t2_);
  for(int i=-3; i<3; i++) {
    for(int j=-3; j<3; j++) {
      if((i+j)%2 != 0) {
        btglRect(i, j, i+1, j+1);
      }
    }
  }

  glPopMatrix();

  // draw black marks
  GLUquadric* quadric = gluNewQuadric();
  if(!quadric) {
    throw(Error("quadric creation failed"));
  }

  glPushMatrix();
  btglTranslate(0, 0, 2*Display::draw_epsilon);

  glColor4fv(Color4(0x14,0x17,0x1c)); // RAL 9017
  for(int i=0;;) { // two steps (x>0 then x<0)
    // vertical side lines
    btglRect(3*SQUARE_SIZE, size_[1]/2, 3*SQUARE_SIZE+0.05_m, -size_[1]/2);
    // secured zone, top
    btglRect(3*SQUARE_SIZE, -2*SQUARE_SIZE, 1*SQUARE_SIZE, -2*SQUARE_SIZE-0.02_m);
    // secured zone, right
    btglRect(1*SQUARE_SIZE, -2*SQUARE_SIZE, 1*SQUARE_SIZE+0.02_m, -3*SQUARE_SIZE);
    // bonus positions (from bottom to top)
    glPushMatrix();
    btglTranslate(0.5*SQUARE_SIZE, -2.5*SQUARE_SIZE, 0);
    gluDisk(quadric, 0, 0.1_m/2, Display::draw_div, Display::draw_div);
    btglTranslate(1*SQUARE_SIZE, 2*SQUARE_SIZE, 0);
    gluDisk(quadric, 0, 0.1_m/2, Display::draw_div, Display::draw_div);
    btglTranslate(0, 2*SQUARE_SIZE, 0);
    gluDisk(quadric, 0, 0.1_m/2, Display::draw_div, Display::draw_div);
    glPopMatrix();

    if(i == 1) {
      break;
    }
    // reverse X
    btglScale(-1,1,1);
    i = 1;
  }

  glPopMatrix();

  gluDeleteQuadric(quadric);
}


const btScalar Magnet::RADIUS = 0.02_m; //note: must be < MagnetPawn::HEIGHT/2
btSphereShape Magnet::shape_(RADIUS);
// User defined constraint type for magnet joints.
#define EUROBOT2011_MAGNET_CONSTRAINT_TYPE  (0x20110001)


Magnet::Magnet(): btRigidBody(btRigidBodyConstructionInfo(0,NULL,&shape_)),
    physics_(NULL)
{
  // small mass to make it a kinetic object
  btVector3 inertia;
  shape_.calculateLocalInertia(0.01, inertia);
  setupRigidBody( btRigidBodyConstructionInfo(0.01,NULL,&shape_,inertia) );

  setCollisionFlags(getCollisionFlags() | CF_NO_CONTACT_RESPONSE);
}

Magnet::~Magnet()
{
  if(enabled()) {
    disable();  // release objects
  }
}

void Magnet::enable(Physics* ph)
{
  if(enabled()) {
    throw(Error("magnet is already enabled"));
  }
  physics_ = ph;
}

void Magnet::disable()
{
  if(!enabled()) {
    throw(Error("magnet is not enabled"));
  }
  // release objects
  for(int i=getNumConstraintRefs()-1; i>=0; i--) {
    btTypedConstraint* constraint = getConstraintRef(i);
    if(constraint->getUserConstraintType() == EUROBOT2011_MAGNET_CONSTRAINT_TYPE) {
      physics_->getWorld()->removeConstraint(constraint);
      delete constraint;
    }
  }
  physics_ = NULL;
}

bool Magnet::checkCollideWithOverride(btCollisionObject* co)
{
  if(!enabled()) {
    return false;
  }

  //XXX use collision flags or something to avoid dynamic_cast
  //XXX This callback is used to detect magnets close from each other, not to
  // known whether an object should collide. They may be a more appropriated
  // way to do this.
  Magnet* o = dynamic_cast<Magnet*>(co);
  if(!o || !o->enabled()) {
    return false;
  }
  // check if objects are close enough
  const btScalar d = (getCenterOfMassTransform().getOrigin() - o->getCenterOfMassTransform().getOrigin()).length();
  if(d > 2*RADIUS) {
    return false;
  }

  // check if object is already constrained
  for(int i=0; i < getNumConstraintRefs(); i++) {
    btTypedConstraint* constraint = getConstraintRef(i);
    if(o == &constraint->getRigidBodyA() || o == &constraint->getRigidBodyB()) {
      return false;
    }
  }

  // new constraint
  btGeneric6DofConstraint* constraint = new btGeneric6DofConstraint(
      *this, *o, btTransform::getIdentity(), btTransform::getIdentity(), true);
  constraint->setUserConstraintType(EUROBOT2011_MAGNET_CONSTRAINT_TYPE);
  physics_->getWorld()->addConstraint(constraint, true);

  return false;
}


const btScalar MagnetPawn::RADIUS = 0.1_m;
const btScalar MagnetPawn::HEIGHT = 0.05_m;

MagnetPawn::MagnetPawn(btCollisionShape* sh, btScalar mass):
    OSimple(sh, mass)
{
  magnet_links_[0] = NULL;
  magnet_links_[1] = NULL;
}

MagnetPawn::~MagnetPawn() {}

void MagnetPawn::addToWorld(Physics* physics)
{
  OSimple::addToWorld(physics);
  for(int i=0; i<2; i++) {
    physics_->getWorld()->addRigidBody(&magnets_[i]);
    btTransform tr = btTransform::getIdentity();
    tr.getOrigin().setZ( (i==0 ? +1 : -1) * HEIGHT/2 );
    magnet_links_[i] = new btGeneric6DofConstraint(*this, magnets_[i], tr, btTransform::getIdentity(), true);
    magnets_[i].setCenterOfMassTransform(getCenterOfMassTransform());
    physics_->getWorld()->addConstraint(magnet_links_[i], true);
    magnets_[i].enable(physics_);
  }
}

void MagnetPawn::removeFromWorld()
{
  for(int i=0; i<2; i++) {
    magnets_[i].disable();
    physics_->getWorld()->removeRigidBody(&magnets_[i]);
    physics_->getWorld()->removeConstraint(magnet_links_[i]);
    magnet_links_[i] = NULL;
  }
  OSimple::removeFromWorld();
}

void MagnetPawn::setTrans(const btTransform& tr)
{
  OSimple::setTrans(tr);
  for(int i=0; i<2; i++) {
    assert( magnet_links_[i] != NULL );
    magnets_[i].setCenterOfMassTransform(magnet_links_[i]->getFrameOffsetA() * tr);
  }
}


Galipeur2011::Galipeur2011(btScalar m): Galipeur(m)
{
  // not a static const to avoid issues of init order of globals
  const btVector3 arm_pos( D_SIDE-0.03_m, 0, MagnetPawn::HEIGHT*2-Z_MASS );
  const btVector3 up(0,0,1);
  const btScalar angles[GALIPEUR2011_ARM_NB] = { -M_PI/3, +M_PI/3 };
  for(unsigned int i=0; i<GALIPEUR2011_ARM_NB; i++) {
    btMatrix3x3 m;
    m.setEulerZYX( 0, 0, -angles[i] );
    btTransform tr( m, arm_pos.rotate(up, M_PI_2-angles[i]) );
    arms_[i] = new PawnArm(this, tr);
  }
}

Galipeur2011::~Galipeur2011()
{
  delete arms_[0];
  delete arms_[1];
}

void Galipeur2011::addToWorld(Physics* physics)
{
  Galipeur::addToWorld(physics);
  for(unsigned int i=0; i<GALIPEUR2011_ARM_NB; i++) {
    arms_[i]->addToWorld();
  }
}

void Galipeur2011::removeFromWorld()
{
  for(unsigned int i=0; i<GALIPEUR2011_ARM_NB; i++) {
    arms_[i]->removeFromWorld();
  }
  Galipeur::removeFromWorld();
}


void Galipeur2011::draw(Display* d) const
{
  Galipeur::draw(d);
  for(unsigned int i=0; i<GALIPEUR2011_ARM_NB; i++) {
    arms_[i]->draw(d);
  }
}

void Galipeur2011::setTrans(const btTransform& tr)
{
  Galipeur::setTrans(tr);
  for(unsigned int i=0; i<GALIPEUR2011_ARM_NB; i++) {
    arms_[i]->resetTrans();
  }
}

void Galipeur2011::asserv()
{
  Galipeur::asserv();
  for(unsigned int i=0; i<2; i++) {
    arms_[i]->asserv();
  }
}


const btScalar Galipeur2011::PawnArm::RADIUS = 0.02_m;
const btScalar Galipeur2011::PawnArm::LENGTH = 0.14_m;
const btScalar Galipeur2011::PawnArm::MASS = 0.1;
const btScalar Galipeur2011::PawnArm::ANGLE_MIN = M_PI*0.1;
const btScalar Galipeur2011::PawnArm::ANGLE_MAX = M_PI_2+M_PI*0.07;

btCapsuleShape Galipeur2011::PawnArm::shape_( RADIUS, LENGTH );

Galipeur2011::PawnArm::PawnArm(Galipeur2011* robot, const btTransform& tr):
    btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL)),
    robot_(robot), robot_tr_(tr)
{
  btVector3 inertia;
  shape_.calculateLocalInertia(MASS, inertia);
  setupRigidBody( btRigidBodyConstructionInfo(MASS,NULL,&shape_,inertia) );

  btTransform tr2 = btTransform::getIdentity();
  tr2.setOrigin( -btVector3(0, LENGTH/2, 0) );
  robot_link_ = new btSliderConstraint(*robot_->body_, *this, robot_tr_, tr2, true);
  robot_link_->setLowerLinLimit(0);
  robot_link_->setUpperLinLimit(0);
  robot_link_->setLowerAngLimit(M_PI_2-ANGLE_MIN);
  robot_link_->setUpperAngLimit(M_PI_2-ANGLE_MIN); // init: raised position
  robot_link_->setPoweredAngMotor(true);
  robot_link_->setTargetAngMotorVelocity(0);
  robot_link_->setMaxAngMotorForce(100); // don't limit acceleration

  tr2 = btTransform::getIdentity();
  tr2.setOrigin( btVector3(0, LENGTH/2, -RADIUS) );
  magnet_link_ = new btGeneric6DofConstraint(*this, magnet_, tr2, btTransform::getIdentity(), true);

  resetTrans();
}

Galipeur2011::PawnArm::~PawnArm()
{
  delete robot_link_;
  delete magnet_link_;
}


void Galipeur2011::PawnArm::raise()
{
  robot_link_->setLowerAngLimit(M_PI_2-ANGLE_MAX);
  robot_link_->setUpperAngLimit(M_PI_2-ANGLE_MIN);
  robot_link_->setTargetAngMotorVelocity( -robot_->arm_av_ );
}

void Galipeur2011::PawnArm::lower()
{
  robot_link_->setLowerAngLimit(M_PI_2-ANGLE_MAX);
  robot_link_->setUpperAngLimit(M_PI_2-ANGLE_MIN);
  robot_link_->setTargetAngMotorVelocity( +robot_->arm_av_ );
}

void Galipeur2011::PawnArm::grab()
{
  if(!magnet_.enabled()) {
    magnet_.enable( robot_->physics_ );
  }
}

void Galipeur2011::PawnArm::release()
{
  if(magnet_.enabled()) {
    magnet_.disable();
  }
}

void Galipeur2011::PawnArm::asserv()
{
  const btScalar threshold = 0.01;
  const btScalar av = robot_link_->getTargetAngMotorVelocity();
  if(av < 0) {
    if(btFabs(M_PI_2-ANGLE_MIN - angle()) < threshold) {
      robot_link_->setLowerAngLimit(M_PI_2-ANGLE_MIN);
      robot_link_->setUpperAngLimit(M_PI_2-ANGLE_MIN);
      robot_link_->setTargetAngMotorVelocity(0);
    }
  } else if(av > 0) {
    if(btFabs(M_PI_2-ANGLE_MAX - angle()) < threshold) {
      robot_link_->setLowerAngLimit(M_PI_2-ANGLE_MAX);
      robot_link_->setUpperAngLimit(M_PI_2-ANGLE_MAX);
      robot_link_->setTargetAngMotorVelocity(0);
    }
  }
}


void Galipeur2011::PawnArm::draw(Display* d) const
{
  glPushMatrix();
  drawTransform(getCenterOfMassTransform());
  if(d->callOrCreateDisplayList(&PawnArm::shape_)) {
    Object::drawShape(&PawnArm::shape_);
    d->endDisplayList();
  }
  glPopMatrix();
}

void Galipeur2011::PawnArm::resetTrans()
{
  // default position: raised
  const btTransform raised( btQuaternion(btVector3(1,0,0), M_PI_2-ANGLE_MIN), btVector3(0,0,0) );
  btTransform tr = robot_->body_->getCenterOfMassTransform() * robot_tr_ * raised;
  tr.getOrigin() += btVector3(0, LENGTH/2, 0);
  setCenterOfMassTransform(tr);
  tr.getOrigin() -= btVector3(0, LENGTH, 0);
  magnet_.setCenterOfMassTransform(tr);
}

void Galipeur2011::PawnArm::addToWorld()
{
  btDynamicsWorld* world = robot_->physics_->getWorld();
  world->addRigidBody(this);
  world->addRigidBody(&magnet_);
  world->addConstraint(robot_link_, true);
  world->addConstraint(magnet_link_, true);
  magnet_.enable(robot_->physics_);
}

void Galipeur2011::PawnArm::removeFromWorld()
{
  btDynamicsWorld* world = robot_->physics_->getWorld();
  if(magnet_.enabled()) {
    magnet_.disable();
  }
  world->removeConstraint(magnet_link_);
  world->removeConstraint(robot_link_);
  world->removeRigidBody(&magnet_);
  world->removeRigidBody(this);
}


}

