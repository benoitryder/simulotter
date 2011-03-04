#include "modules/eurobot2011.h"
#include "display.h"
#include "log.h"


namespace eurobot2011
{

const btScalar OGround2011::SQUARE_SIZE = btScale(0.350);
const btScalar OGround2011::START_SIZE = btScale(0.400);

OGround2011::OGround2011():
    OGround(Color4(0x24,0x91,0x40), // RAL 6024
            Color4(0xc7,0x17,0x12), // RAL 3020
            Color4(0x00,0x3b,0x80)) // RAL 5017
{
  setStartSize(START_SIZE);
}

void OGround2011::draw(Display *d) const
{
  glPushMatrix();

  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(this) ) {
    drawBase();
    drawStartingAreas();

    btglTranslate(0, 0, SIZE[2]/2);

    // draw the checkerboard
    glPushMatrix();
    btglNormal3(0.0, 0.0, 1.0);

    btglTranslate(0, 0, Display::draw_epsilon);
    btglScale(SQUARE_SIZE, SQUARE_SIZE, 1);

    // 6 lines, 6 columns, 1 square out of 2 (for each color)
    int i, j;

    glColor4fv(color_t1_);
    for( i=-3; i<3; i++ ) {
      for( j=-3; j<3; j++ ) {
        if( (i+j)%2 == 0 ) {
          btglRect(i, j, i+1, j+1);
        }
      }
    }

    glColor4fv(color_t2_);
    for( i=-3; i<3; i++ ) {
      for( j=-3; j<3; j++ ) {
        if( (i+j)%2 != 0 ) {
          btglRect(i, j, i+1, j+1);
        }
      }
    }

    glPopMatrix();

    // draw black marks
    GLUquadric *quadric = gluNewQuadric();
    if( quadric == NULL ) {
      throw(Error("quadric creation failed"));
    }

    glPushMatrix();
    btglTranslate(0, 0, 2*Display::draw_epsilon);

    glColor4fv(Color4(0x14,0x17,0x1c)); // RAL 9017
    for( i=0;; ) { // two steps (x>0 then x<0)
      // vertical side lines
      btglRect(3*SQUARE_SIZE, SIZE[1]/2, 3*SQUARE_SIZE+btScale(0.05), -SIZE[1]/2);
      // secured zone, top
      btglRect(3*SQUARE_SIZE, -2*SQUARE_SIZE, 1*SQUARE_SIZE, -2*SQUARE_SIZE-btScale(0.02));
      // secured zone, right
      btglRect(1*SQUARE_SIZE, -2*SQUARE_SIZE, 1*SQUARE_SIZE+btScale(0.02), -3*SQUARE_SIZE);
      // bonus positions (from bottom to top)
      glPushMatrix();
      btglTranslate(0.5*SQUARE_SIZE, -2.5*SQUARE_SIZE, 0);
      gluDisk(quadric, 0, btScale(0.1/2), Display::draw_div, Display::draw_div);
      btglTranslate(1*SQUARE_SIZE, 2*SQUARE_SIZE, 0);
      gluDisk(quadric, 0, btScale(0.1/2), Display::draw_div, Display::draw_div);
      btglTranslate(0, 2*SQUARE_SIZE, 0);
      gluDisk(quadric, 0, btScale(0.1/2), Display::draw_div, Display::draw_div);
      glPopMatrix();

      if( i == 1 ) {
        break;
      }
      // reverse X
      btglScale(-1,1,1);
      i = 1;
    }

    glPopMatrix();

    gluDeleteQuadric(quadric);

    d->endDisplayList();
  }

  glPopMatrix();
}


Galipeur2011::Galipeur2011(btScalar m): Galipeur(m)
{
  // not a static const to avoid issues of init order of globals
  const btVector3 arm_pos(D_SIDE, 0, btScale(0.05)+PawnArm::RADIUS+btScalar(0.02)-Z_MASS);
  const btVector3 up(0,0,1);
  const btScalar angles[sizeof(arms_)/sizeof(*arms_)] = { -M_PI/3, +M_PI/3 };
  unsigned int i;
  for( i=0; i<sizeof(arms_)/sizeof(*arms_); i++ ) {
    btTransform tr( btQuaternion(up, -angles[i]), arm_pos.rotate(up, M_PI_2-angles[i]) );
    arms_[i] = new PawnArm(this, tr);
  }
}

Galipeur2011::~Galipeur2011()
{
  delete arms_[0];
  delete arms_[1];
}

void Galipeur2011::addToWorld(Physics *physics)
{
  btDynamicsWorld *world = physics->getWorld();
  unsigned int i;
  for( i=0; i<sizeof(arms_)/sizeof(*arms_); i++ ) {
    world->addRigidBody(arms_[i]);
    world->addConstraint(arms_[i]->robot_link_, true);
  }
  Galipeur::addToWorld(physics);
}

void Galipeur2011::removeFromWorld()
{
  btDynamicsWorld *world = physics_->getWorld();
  Galipeur::removeFromWorld();
  unsigned int i;
  for( i=0; i<sizeof(arms_)/sizeof(*arms_); i++ ) {
    world->removeConstraint(arms_[i]->robot_link_);
    world->removeRigidBody(arms_[i]);
  }
}

void Galipeur2011::draw(Display *d) const
{
  Galipeur::draw(d);

  unsigned int i;
  for( i=0; i<sizeof(arms_)/sizeof(*arms_); i++ ) {
    arms_[i]->draw(d);
  }
}

void Galipeur2011::setTrans(const btTransform &tr)
{
  Galipeur::setTrans(tr);
  unsigned int i;
  for( i=0; i<sizeof(arms_)/sizeof(*arms_); i++ ) {
    arms_[i]->resetTrans();
  }
}


const btScalar Galipeur2011::PawnArm::RADIUS = btScale(0.02);
const btScalar Galipeur2011::PawnArm::LENGTH = btScale(0.1);
const btScalar Galipeur2011::PawnArm::MASS = 0.1;

btCapsuleShape Galipeur2011::PawnArm::shape_( RADIUS, LENGTH );

Galipeur2011::PawnArm::PawnArm(Galipeur2011 *robot, const btTransform &tr):
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
  robot_link_->setLowerAngLimit(0);
  robot_link_->setUpperAngLimit(M_PI_2);
  robot_link_->setPoweredAngMotor(true);
  robot_link_->setTargetAngMotorVelocity(0);
  robot_link_->setMaxAngMotorForce(100); // don't limit acceleration

  this->resetTrans();
}

Galipeur2011::PawnArm::~PawnArm()
{
  delete robot_link_;
}

void Galipeur2011::PawnArm::draw(Display *d) const
{
  glPushMatrix();
  drawTransform(this->getCenterOfMassTransform());
  if( d->callOrCreateDisplayList(&PawnArm::shape_) ) {
    Object::drawShape(&PawnArm::shape_);
    d->endDisplayList();
  }
  glPopMatrix();
}

void Galipeur2011::PawnArm::resetTrans()
{
  btTransform tr = robot_->body_->getCenterOfMassTransform() * robot_tr_;
  tr.getOrigin() += btVector3(0, LENGTH/2, 0);
  this->setCenterOfMassTransform(tr);
}


}

