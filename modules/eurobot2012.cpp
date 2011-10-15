#include "modules/eurobot2012.h"
#include "display.h"
#include "log.h"


namespace eurobot2012
{

const btVector2 OGround2012::SIZE = btScale(btVector2(3.0, 2.1));
const btScalar OGround2012::START_SIZE = btScale(0.500);

OGround2012::OGround2012():
    OGround(SIZE,
            Color4(0x29,0x73,0xb8), // RAL 5012
            Color4(0x7d,0x1f,0x7a), // RAL 4008
            Color4(0xa3,0x17,0x1a)) // RAL 3001
{
  setStartSize(START_SIZE);
}

void OGround2012::draw(Display *d) const
{
  glPushMatrix();

  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(this) ) {
    drawBase();
    drawStartingAreas();

    btglTranslate(0, 0, size_[2]/2);
    btglTranslate(0, 0, Display::draw_epsilon);

    GLUquadric *quadric = gluNewQuadric();
    if( quadric == NULL ) {
      throw(Error("quadric creation failed"));
    }

    const Color4 color_black(0x03,0x05,0x0a); // RAL 9005
    const Color4 color_boat(0x6e,0x3b,0x3a); // RAL 8002
    const Color4 color_sand(0xfc,0xbd,0x1f); // RAL 1023
    const Color4 color_jungle(0x4f,0xa8,0x33); // RAL 6018

    // black lines
    glColor4fv(color_black);
    {
      const btScalar x0 = size_.x()/2 - btScale(0.5+0.15);
      const btScalar y0 = size_.y()/2 - btScale(0.5-0.05);
      const btScalar x1 = size_.x()/2 - btScale(0.5);
      const btScalar y1 = -size_.y()/2;
      const btScalar width = btScale(0.02);
      btglRect(x0, y0, x0+width, y1);
      btglRect(x0, y0, x1, y0-width);
      btglRect(-x0, y0, -x0-width, y1);
      btglRect(-x0, y0, -x1, y0-width);
    }

    // boats
    glColor4fv(color_boat);
    glBegin(GL_QUADS);
    {
      const btScalar x0 = size_.x()/2;
      const btScalar y0 = -size_.y()/2;
      const btScalar x1 = x0-btScale(0.325);
      const btScalar x2 = x0-btScale(0.4);
      const btScalar y1 = size_.y()/2-btScale(0.5+0.018);
      btglVertex3(x0, y0, 0);
      btglVertex3(x1, y0, 0);
      btglVertex3(x2, y1, 0);
      btglVertex3(x0, y1, 0);
      btglVertex3(-x0, y0, 0);
      btglVertex3(-x1, y0, 0);
      btglVertex3(-x2, y1, 0);
      btglVertex3(-x0, y1, 0);
    }
    glEnd();

    // peanut island: draw sand first, then water and jungle above it
    // sand: two disks and two arcs to fill the middle part
    glColor4fv(color_sand);
    btglTranslate(-btScale(0.8/2), 0, 0);
    gluDisk(quadric, 0, btScale(0.3), 2*Display::draw_div, 2*Display::draw_div);
    btglTranslate(2*btScale(0.8/2), 0, 0);
    gluDisk(quadric, 0, btScale(0.3), 2*Display::draw_div, 2*Display::draw_div);
    //TODO Y offset for the inner curve is not known
    const btScalar curve_y = btScale(0.745);
    btglTranslate(-btScale(0.8/2), -curve_y, 0);
    gluPartialDisk(quadric, btScale(0.55), btScale(0.9), Display::draw_div, Display::draw_div, -30, 60);
    btglTranslate(0, 2*curve_y, 0);
    gluPartialDisk(quadric, btScale(0.55), btScale(0.9), Display::draw_div, Display::draw_div, 150, 60);
    // jungle
    glColor4fv(color_jungle);
    btglTranslate(-btScale(0.8/2), -curve_y, Display::draw_epsilon);
    gluDisk(quadric, 0, btScale(0.2), 2*Display::draw_div, 2*Display::draw_div);
    btglTranslate(2*btScale(0.8/2), 0, 0);
    gluDisk(quadric, 0, btScale(0.2), 2*Display::draw_div, 2*Display::draw_div);

    // map island
    btglTranslate(-btScale(0.8/2), size_.y()/2, -Display::draw_epsilon); // also restore Z offset
    glColor4fv(color_jungle);
    gluPartialDisk(quadric, 0, btScale(0.6/2), Display::draw_div, Display::draw_div, 90, 180);
    glColor4fv(color_sand);
    gluPartialDisk(quadric, btScale(0.6/2), btScale(0.8/2), Display::draw_div, Display::draw_div, 90, 180);

    gluDeleteQuadric(quadric);

    d->endDisplayList();
  }

  glPopMatrix();
}


const btVector3 OBullion::SIZE = btScale(btVector3(0.150, 0.070, 0.0485));
const btScalar OBullion::MASS = 0.100;
const btScalar OBullion::A_SLOPE = 75*M_PI/180;
SmartPtr<btConvexHullShape> OBullion::shape_(new btConvexHullShape());

OBullion::OBullion()
{
  // First instance: initialize shape
  if( shape_->getNumPoints() == 0 ) {
    const btScalar z = SIZE.z()/2;
    const btScalar wslope = SIZE.z() * btCos(A_SLOPE);
    const btVector2 p0 = btVector2(SIZE.x()/2, SIZE.y()/2);
    const btVector2 p1 = btVector2(SIZE.x()/2-wslope, SIZE.y()/2-wslope);
    shape_->addPoint( btVector3( p0.x(),  p0.y(), -z) );
    shape_->addPoint( btVector3( p0.x(), -p0.y(), -z) );
    shape_->addPoint( btVector3(-p0.x(), -p0.y(), -z) );
    shape_->addPoint( btVector3(-p0.x(),  p0.y(), -z) );
    shape_->addPoint( btVector3( p1.x(),  p1.y(),  z) );
    shape_->addPoint( btVector3( p1.x(), -p1.y(),  z) );
    shape_->addPoint( btVector3(-p1.x(), -p1.y(),  z) );
    shape_->addPoint( btVector3(-p1.x(),  p1.y(),  z) );
    shape_->setMargin(shape_->getMargin()/2);
  }

  this->setShape(shape_);
  this->setMass(MASS);
  this->setColor(Color4(0xfc,0xbd,0x1f)); // RAL 1023
}


void OBullion::draw(Display *d) const
{
  glColor4fv(color_);
  glPushMatrix();
  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(m_collisionShape) ) {
    // same values as in constructor
    const btScalar z = SIZE.z()/2;
    const btScalar wslope = SIZE.z() * btCos(A_SLOPE);
    const btVector2 p0 = btVector2(SIZE.x()/2, SIZE.y()/2);
    const btVector2 p1 = btVector2(SIZE.x()/2-wslope, SIZE.y()/2-wslope);

    glBegin(GL_QUADS);
    // bottom
    btglNormal3(0.0, 0.0, -1.0);
    btglVertex3( p0.x(),  p0.y(), -z);
    btglVertex3( p0.x(), -p0.y(), -z);
    btglVertex3(-p0.x(), -p0.y(), -z);
    btglVertex3(-p0.x(),  p0.y(), -z);
    // top
    btglNormal3(0.0, 0.0, 1.0);
    btglVertex3( p1.x(),  p1.y(),  z);
    btglVertex3( p1.x(), -p1.y(),  z);
    btglVertex3(-p1.x(), -p1.y(),  z);
    btglVertex3(-p1.x(),  p1.y(),  z);
    // front
    btglNormal3(0.0, -1.0, 0.0);
    btglVertex3(-p0.x(), -p0.y(), -z);
    btglVertex3( p0.x(), -p0.y(), -z);
    btglVertex3( p1.x(), -p1.y(),  z);
    btglVertex3(-p1.x(), -p1.y(),  z);
    // back
    btglNormal3(0.0, 1.0, 0.0);
    btglVertex3(-p0.x(),  p0.y(), -z);
    btglVertex3( p0.x(),  p0.y(), -z);
    btglVertex3( p1.x(),  p1.y(),  z);
    btglVertex3(-p1.x(),  p1.y(),  z);
    // left
    btglNormal3(-1.0, 0.0, 0.0);
    btglVertex3(-p0.x(),  p0.y(), -z);
    btglVertex3(-p0.x(), -p0.y(), -z);
    btglVertex3(-p1.x(), -p1.y(),  z);
    btglVertex3(-p1.x(),  p1.y(),  z);
    // right
    btglNormal3(1.0, 0.0, 0.0);
    btglVertex3( p0.x(),  p0.y(), -z);
    btglVertex3( p0.x(), -p0.y(), -z);
    btglVertex3( p1.x(), -p1.y(),  z);
    btglVertex3( p1.x(),  p1.y(),  z);

    glEnd();
    d->endDisplayList();
  }

  glPopMatrix();
}


const btScalar OCoin::DISC_HEIGHT = btScale(0.002);
const btScalar OCoin::RADIUS = btScale(0.120/2);
const btScalar OCoin::INNER_RADIUS = btScale(0.015/2);
const btScalar OCoin::CUBE_SIZE = btScale(0.018);
const btScalar OCoin::CUBE_OFFSET = btScale(0.0315);
const btScalar OCoin::MASS = 0.030;
SmartPtr<btCompoundShape> OCoin::shape_;
btCylinderShapeZ OCoin::shape_disc_(btVector3(RADIUS, RADIUS, DISC_HEIGHT/2));
btBoxShape OCoin::shape_cube_(btVector3(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE)/2);

OCoin::OCoin(bool white)
{
  // First instance: initialize shape
  if( shape_ == NULL ) {
    shape_disc_.setMargin(shape_disc_.getMargin()/4);
    btTransform tr = btTransform::getIdentity();
    shape_ = new btCompoundShape();
    shape_->addChildShape(tr, &shape_disc_);
    tr.setOrigin(btVector3(CUBE_OFFSET+CUBE_SIZE/2, 0, -(CUBE_SIZE+DISC_HEIGHT)/2));
    shape_->addChildShape(tr, &shape_cube_);
  }

  this->setShape(shape_);
  this->setMass(MASS);
  this->setColor(white ? Color4::white() : Color4::black());
}

void OCoin::draw(Display *d) const
{
  glPushMatrix();

  drawTransform(m_worldTransform);
  glColor4fv(color_);

  if( d->callOrCreateDisplayList(m_collisionShape) ) {
    // disc: outer/inner cylinders, bottom/bottom disks
    btglTranslate(0, 0, -DISC_HEIGHT/2);
    GLUquadric *quadric = gluNewQuadric();
    gluCylinder(quadric, RADIUS, RADIUS, DISC_HEIGHT, Display::draw_div, 1);
    gluQuadricOrientation(quadric, GLU_INSIDE);
    gluCylinder(quadric, INNER_RADIUS, INNER_RADIUS, DISC_HEIGHT, Display::draw_div/2, 1);
    gluDisk(quadric, INNER_RADIUS, RADIUS, Display::draw_div, 1);
    gluQuadricOrientation(quadric, GLU_OUTSIDE);
    btglTranslate(0, 0, DISC_HEIGHT);
    gluDisk(quadric, INNER_RADIUS, RADIUS, Display::draw_div, 1);
    gluDeleteQuadric(quadric);
    // cube
    btglTranslate(CUBE_OFFSET+CUBE_SIZE/2, 0, -DISC_HEIGHT-CUBE_SIZE/2);
    glutSolidCube(CUBE_SIZE);

    d->endDisplayList();
  }

  glPopMatrix();
}


}

