#include "modules/eurobot2013.h"
#include "display.h"
#include "graphics.h"
#include "log.h"

namespace eurobot2013 {

static const Color4 color_t1(0x00,0x3b,0x80); // RAL 5017
static const Color4 color_t2(0xa3,0x17,0x1a); // RAL 3001
static const Color4 color_neutral(0xfc,0xff,0xff); // RAL 9016
static const Color4 color_black(0x03,0x05,0x0a); // RAL 9005


const btVector2 OGround2013::SIZE = btVector2(3.0_m, 2.0_m);
const btScalar OGround2013::SQUARE_SIZE = 0.400_m;

OGround2013::OGround2013():
    OGround(SIZE, Color4(0xfc,0xbd,0x1f))
{
}

void OGround2013::drawDisplayList() const
{
  OGround::drawDisplayList();

  btglNormal3(0.0, 0.0, 1.0);

  const btScalar SX = size_.x()/2;
  const btScalar SY = size_.y()/2;

  btglTranslate(0, 0, size_.z()/2+Display::draw_epsilon);

  // sideboards are not part of the ground
  // draw team-colored rectangles then white squares on them
  glColor4fv(color_t1);
  btglRect(-SX, SY, -SX+SQUARE_SIZE, -SY);
  glColor4fv(color_t2);
  btglRect(+SX, SY, +SX-SQUARE_SIZE, -SY);

  btglTranslate(0, 0, Display::draw_epsilon);

  glColor4fv(color_neutral);
  btglRect(-SX, +SQUARE_SIZE*1.5, -SX+SQUARE_SIZE, +SQUARE_SIZE*0.5);
  btglRect( SX, +SQUARE_SIZE*1.5, +SX-SQUARE_SIZE, +SQUARE_SIZE*0.5);
  btglRect(-SX, -SQUARE_SIZE*1.5, -SX+SQUARE_SIZE, -SQUARE_SIZE*0.5);
  btglRect( SX, -SQUARE_SIZE*1.5, +SX-SQUARE_SIZE, -SQUARE_SIZE*0.5);

  // black lines
  {
    glColor4fv(color_black);
    // black lines are above white squares
    btglTranslate(0, 0, Display::draw_epsilon);

    const btScalar W = 0.020_m; // demi line width

    // lines coordinates
    const btScalar x0 = 0.900_m; // outer
    const btScalar x1 = 0.300_m; // inner
    const btScalar y0 = -SY+1.400_m; // top
    const btScalar y1 = -SY+1.300_m; // middle
    const btScalar y2 = -SY+0.300_m; // bottom

    const btScalar R = 0.150_m; // corner radius
    const btScalar Ri = R-W; // inner radius
    const btScalar Ro = R+W; // outer radius

    // horizontal straight lines (top to bottom, left to right)
    btglRect(-SX, y0-W, -x0-R, y0+W);
    btglRect(+SX, y0-W, +x0+R, y0+W);
    btglRect(-x0+R, y1-W, +x0-R, y1+W);
    btglRect(-x0+R, y2-W, +x0-R, y2+W);
    // vertical straight lines (left to right)
    btglRect(-x0-W, y0-R, -x0+W, -SY);
    btglRect(-x1-W, y2, -x1+W, -SY);
    btglRect(+x1-W, y2, +x1+W, -SY);
    btglRect(+x0-W, y0-R, +x0+W, -SY);

    GLUquadric* quadric = gluNewQuadric();
    if(!quadric) {
      throw(Error("quadric creation failed"));
    }

    // external corners (left, right)
    btglTranslate(-x0-R, y0-R, 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 0, 90);
    btglTranslate(2*(x0+R), 0, 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 0, -90);
    // inner corners (top right, top left, bottom left, bottom right)
    btglTranslate(-2*R, y1-y0, 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 0, 90);
    btglTranslate(-2*(x0-R), 0, 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 0, -90);
    btglTranslate(0, (y2+R)-(y1-R), 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 180, 90);
    btglTranslate(2*(x0-R), 0, 0);
    gluPartialDisk(quadric, Ri, Ro, Display::draw_div, Display::draw_div, 180, -90);

    gluDeleteQuadric(quadric);
  }
}


constexpr unsigned int OCake::BASKET_SLICES;
constexpr btScalar OCake::LEVEL_HEIGHT;
constexpr btScalar OCake::LEVEL_RADIUS;
constexpr btScalar OCake::BASE_RADIUS;
constexpr btScalar OCake::BASKET_HEIGHT;
constexpr btScalar OCake::BASKET_RADIUS;
constexpr btScalar OCake::BASKET_WIDTH;
constexpr btScalar OCake::BASKET_WALL_HEIGHT;
constexpr btScalar OCake::BACK_HEIGHT;
constexpr btScalar OCake::BACK_WIDTH;

SmartPtr<btCompoundShape> OCake::shape_;
std::vector<SmartPtr<btCylinderShapeZ>> OCake::shape_levels_;
btCompoundShape OCake::shape_basket_;
btBoxShape OCake::shape_basket_slice_(btVector3(
        2*BASKET_RADIUS*btSin(M_PI_2/BASKET_SLICES),
        BASKET_WIDTH, BASKET_HEIGHT)/2);
btBoxShape OCake::shape_basket_wall_(btVector3(BASKET_WIDTH, BASKET_RADIUS, BASKET_WALL_HEIGHT)/2);
btBoxShape OCake::shape_back_(btVector3(2*BASKET_RADIUS, BACK_WIDTH, BACK_HEIGHT)/2);


OCake::OCake()
{
  // first instance: initialize shape
  if(!shape_) {
    shape_ = new btCompoundShape();
    btTransform tr = btTransform::getIdentity();

    // level shapes
    for(size_t i=0; i<3; ++i) {
      const btScalar r = BASE_RADIUS - i*LEVEL_RADIUS;
      const btScalar h = (i+1)*LEVEL_HEIGHT;
      shape_levels_.push_back(new btCylinderShapeZ(btVector3(r, r, h/2)));
      tr.setOrigin(btVector3(0, 0, h/2));
      shape_->addChildShape(tr, shape_levels_[i]);
    }

    // basket shape
    tr.setOrigin(btVector3(0, BASKET_RADIUS-BASKET_WIDTH, 0));
    for(unsigned int i=0; i<=BASKET_SLICES; i++) {
      btTransform tr2(btQuaternion(btVector3(0,0,1), i*-M_PI/BASKET_SLICES-M_PI_2));
      shape_basket_.addChildShape(tr2 * tr, &shape_basket_slice_);
    }
    // wall
    tr.setOrigin(btVector3(0, -BASKET_RADIUS/2, 0));
    shape_basket_.addChildShape(tr, &shape_basket_wall_);

    tr.setOrigin(btVector3(0, 0, 3*LEVEL_HEIGHT+BASKET_HEIGHT/2));
    shape_->addChildShape(tr, &shape_basket_);

    // back
    tr.setOrigin(btVector3(0, BACK_WIDTH/2, BACK_HEIGHT/2));
    shape_->addChildShape(tr, &shape_back_);
  }

  setColor(Color4(0xe8,0x9c,0xb5));
  setShape(shape_);
  setPos(btVector3(0, OGround2013::SIZE.y()/2, 0));
}


void OCake::draw(Display* d) const
{
  glPushMatrix();
  drawTransform(m_worldTransform);

  if(d->callOrCreateDisplayList(this)) {
    glColor4fv(color_); // color should not change, ok to be in display list

    // draw partial cylinders for levels
    {
      for(size_t lvl=0; lvl<3; ++lvl) {
        const btScalar r = BASE_RADIUS - lvl*LEVEL_RADIUS;
        const btScalar h = (lvl+1)*LEVEL_HEIGHT;

        // side / upper face
        graphics::drawCylinder(r, h, Display::draw_div, M_PI, M_PI);
        graphics::drawDisk(0, r, h, Display::draw_div, M_PI, M_PI);

        // back
        btglNormal3(0, 1, 0);
        glBegin(GL_QUADS);
          btglVertex3(-r, 0, 0);
          btglVertex3(+r, 0, 0);
          btglVertex3(+r, 0, h);
          btglVertex3(-r, 0, h);
        glEnd();
      }
    }

    // draw back
    glPushMatrix();
      btglTranslate(0, BACK_WIDTH/2, BACK_HEIGHT/2);
      glColor4fv(color_neutral);
      drawShape(&shape_back_);
    glPopMatrix();

    // team color panels
    btglNormal3(0, -1, 0);
    glBegin(GL_QUADS);
      glColor4fv(color_t1);
      btglVertex3(-BASKET_RADIUS, -Display::draw_epsilon, 0.300_m);
      btglVertex3(-BASKET_RADIUS, -Display::draw_epsilon, 0.500_m);
      btglVertex3(0, -Display::draw_epsilon, 0.500_m);
      btglVertex3(0, -Display::draw_epsilon, 0.300_m);
      glColor4fv(color_t2);
      btglVertex3(0, -Display::draw_epsilon, 0.300_m);
      btglVertex3(0, -Display::draw_epsilon, 0.500_m);
      btglVertex3(+BASKET_RADIUS, -Display::draw_epsilon, 0.500_m);
      btglVertex3(+BASKET_RADIUS, -Display::draw_epsilon, 0.300_m);
    glEnd();

    // black tape on the basket
    {
      const btScalar r = BASKET_RADIUS + Display::draw_epsilon;
      glColor4fv(Color4::black);
      btglTranslate(0, 0, 3*LEVEL_HEIGHT);
      graphics::drawCylinder(r, 0.025_m, Display::draw_div, M_PI, M_PI);
      btglTranslate(0, 0, BASKET_HEIGHT-2*0.025_m);
      graphics::drawCylinder(r, 0.025_m, Display::draw_div, M_PI, M_PI);
    }

    d->endDisplayList();
  }

  glPopMatrix();
}


void OCake::drawLast(Display* d) const
{
  glPushMatrix();
  drawTransform(m_worldTransform);

  if(d->callOrCreateDisplayList(&shape_basket_)) {
    glColor4fv(Color4::plexi);

    // basket wall (better to be drawn first)
    btglTranslate(0, -BASKET_RADIUS/2, 3*LEVEL_HEIGHT + BASKET_WALL_HEIGHT/2);
    drawShape(&shape_basket_wall_);

    // basket cylinder: outer / inner / top
    const btScalar r0 = BASKET_RADIUS-BASKET_WIDTH;
    const btScalar r1 = BASKET_RADIUS;
    btglTranslate(0, BASKET_RADIUS/2, -BASKET_WALL_HEIGHT/2);
    graphics::drawCylinder(r0, BASKET_HEIGHT, Display::draw_div, M_PI, M_PI);
    graphics::drawCylinder(r1, BASKET_HEIGHT, Display::draw_div, M_PI, M_PI);
    graphics::drawDisk(r0, r1, BASKET_HEIGHT, Display::draw_div, M_PI, M_PI);

    d->endDisplayList();
  }

  glPopMatrix();
}


const btVector3 OGift::SIZE(0.150_m, 0.022_m, 0.200_m);

SmartPtr<btBoxShape> OGift::shape_(new btBoxShape(SIZE/2));

OGift::OGift(): OSimple(shape_, 0.400)
{
  setDamping(0.8, 0.8);
}


const btVector3 OGiftSupport::SIZE(0.374_m, 0.082_m, 0.022_m);

SmartPtr<btCompoundShape> OGiftSupport::shape_;
btBoxShape OGiftSupport::shape_x_(btVector3(SIZE.x(), SIZE.z(), SIZE.z())/2);
btBoxShape OGiftSupport::shape_y_(btVector3(SIZE.z(), SIZE.y(), SIZE.z())/2);

OGiftSupport::OGiftSupport()
{
  // first instance: initialize shape
  if(!shape_) {
    shape_ = new btCompoundShape();
    btTransform tr = btTransform::getIdentity();

    tr.setOrigin(btVector3(0, -SIZE.z()/2, 0));
    shape_->addChildShape(tr, &shape_x_);

    tr.setOrigin(btVector3(-(SIZE.x()-SIZE.z())/2, -SIZE.y()/2, 0));
    shape_->addChildShape(tr, &shape_y_);
    tr.setOrigin(btVector3(0, -SIZE.y()/2, 0));
    shape_->addChildShape(tr, &shape_y_);
    tr.setOrigin(btVector3(+(SIZE.x()-SIZE.z())/2, -SIZE.y()/2, 0));
    shape_->addChildShape(tr, &shape_y_);
  }

  setColor(Color4(0xfc,0xbd,0x1f));
  setShape(shape_);

  initGift(0);
  initGift(1);
}


void OGiftSupport::setTrans(const btTransform& tr)
{
  OSimple::setTrans(tr);
  resetGiftTrans(0);
  resetGiftTrans(1);
}

void OGiftSupport::addToWorld(Physics* physics)
{
  OSimple::addToWorld(physics);
  physics->getWorld()->addRigidBody(&gifts_[0]);
  physics->getWorld()->addRigidBody(&gifts_[1]);
  physics->getWorld()->addConstraint(gift_links_[0].get(), true);
  physics->getWorld()->addConstraint(gift_links_[1].get(), true);
}

void OGiftSupport::removeFromWorld()
{
  physics_->getWorld()->removeConstraint(gift_links_[0].get());
  physics_->getWorld()->removeConstraint(gift_links_[1].get());
  physics_->getWorld()->removeRigidBody(&gifts_[0]);
  physics_->getWorld()->removeRigidBody(&gifts_[1]);
  OSimple::removeFromWorld();
}


void OGiftSupport::initGift(unsigned int n)
{
  if( n >= 2 ) {
    throw Error("invalid call: initGift(%u)", n);
  }
  gifts_[n].setColor(n == 0 ? color_t1 : color_t2);
  btScalar kx = n == 0 ? -1 : 1;
  gift_links_[n] = std::unique_ptr<btHingeConstraint>(
      new btHingeConstraint(
          *this, gifts_[n],
          btVector3(kx*SIZE.x()/4, -0.050_m, 0),
          btVector3(0, 0, -OGift::SIZE.z()/2+0.015_m),
          btVector3(1, 0, 0), btVector3(1, 0, 0))
      );
  resetGiftTrans(n);
}

void OGiftSupport::resetGiftTrans(unsigned int n)
{
  btScalar kx = n == 0 ? -1 : 1;
  btTransform tr(
      btQuaternion(btVector3(1, 0, 0), 0.1*M_PI),
      btVector3(kx*SIZE.x()/4, -0.050_m, -OGift::SIZE.z()/2+0.015_m));
  gifts_[n].setTrans(getTrans() * tr);
}

void OGiftSupport::draw(Display* d) const
{
  OSimple::draw(d);
  gifts_[0].draw(d);
  gifts_[1].draw(d);
}



constexpr unsigned int OGlass::SLICES;
constexpr btScalar OGlass::HEIGHT;
constexpr btScalar OGlass::RADIUS;
constexpr btScalar OGlass::INNER_RADIUS;
constexpr btScalar OGlass::BOTTOM_HEIGHT;
SmartPtr<btCompoundShape> OGlass::shape_;
btBoxShape OGlass::shape_slice_(btVector3(2*RADIUS*btSin(M_PI/SLICES),
                                          RADIUS-INNER_RADIUS, HEIGHT-BOTTOM_HEIGHT)/2);
btCylinderShapeZ OGlass::shape_bottom_(btVector3(RADIUS, RADIUS, BOTTOM_HEIGHT/2));


OGlass::OGlass()
{
  // first instance: initialize shape
  if(!shape_) {
    shape_ = new btCompoundShape();
    btTransform tr = btTransform::getIdentity();

    // hollow cylinder shape
    tr.setOrigin(btVector3(0, INNER_RADIUS, BOTTOM_HEIGHT/2));
    for(unsigned int i=0; i<SLICES; i++) {
      btTransform tr2(btQuaternion(btVector3(0,0,1), i*2*M_PI/SLICES));
      shape_->addChildShape(tr2 * tr, &shape_slice_);
    }

    // bottom
    tr.setOrigin(btVector3(0, 0, (BOTTOM_HEIGHT-HEIGHT)/2));
    shape_->addChildShape(tr, &shape_bottom_);
  }

  setShape(shape_);
  setMass(0.100);
}


void OGlass::draw(Display* d) const
{
  glPushMatrix();
  drawTransform(m_worldTransform);

  if(d->callOrCreateDisplayList(&shape_bottom_)) {
    glColor4fv(Color4::black);
    btglTranslate(0, 0, (BOTTOM_HEIGHT-HEIGHT)/2);
    drawShape(&shape_bottom_);

    d->endDisplayList();
  }

  glPopMatrix();
}

void OGlass::drawLast(Display* d) const
{
  glPushMatrix();
  drawTransform(m_worldTransform);

  if(d->callOrCreateDisplayList(shape_)) {
    glColor4fv(Color4::plexi);
    btglTranslate(0, 0, -(HEIGHT-BOTTOM_HEIGHT)/2);
    drawShape(&shape_bottom_);

    // outer / inner / top
    graphics::drawCylinder(RADIUS, HEIGHT, Display::draw_div);
    graphics::drawCylinder(INNER_RADIUS, HEIGHT, Display::draw_div);
    graphics::drawDisk(INNER_RADIUS, RADIUS, HEIGHT, Display::draw_div);

    d->endDisplayList();
  }

  glPopMatrix();
}


constexpr btScalar OCandleFlame::RADIUS;

SmartPtr<btSphereShape> OCandleFlame::shape_(new btSphereShape(RADIUS));

OCandleFlame::OCandleFlame(): OSimple(shape_, 0.057)
{
  setColor(Color4(0xc6,0xed,0x2c));
}


constexpr btScalar OCandle::HEIGHT;
constexpr btScalar OCandle::RADIUS;

SmartPtr<btCylinderShapeZ> OCandle::shape_(new btCylinderShapeZ(btVector3(RADIUS, RADIUS, HEIGHT/2)));

OCandle::OCandle(): OSimple(shape_)
{
  btTransform tr_a, tr_b;
  tr_a.setIdentity();
  tr_b.setIdentity();
  tr_a.getBasis().setEulerZYX(0, -M_PI_2, 0);
  tr_b.getBasis().setEulerZYX(0, -M_PI_2, 0);
  tr_a.setOrigin(btVector3(0, 0, -HEIGHT/2));

  flame_link_ = std::unique_ptr<btSliderConstraint>(
      new btSliderConstraint(*this, flame_, tr_a, tr_b, true));
}

void OCandle::setTrans(const btTransform& tr)
{
  OSimple::setTrans(tr);
  resetFlameTrans();
}

void OCandle::addToWorld(Physics* physics)
{
  OSimple::addToWorld(physics);
  physics->getWorld()->addRigidBody(&flame_);
  physics->getWorld()->addConstraint(flame_link_.get(), true);
}

void OCandle::removeFromWorld()
{
  physics_->getWorld()->removeConstraint(flame_link_.get());
  physics_->getWorld()->removeRigidBody(&flame_);
  OSimple::removeFromWorld();
}

void OCandle::resetFlameTrans()
{
  btTransform tr = btTransform::getIdentity();
  tr.setOrigin(btVector3(0, 0, HEIGHT/2 + OCandleFlame::RADIUS/2));
  flame_.setTrans(getTrans() * tr);
}

void OCandle::draw(Display* d) const
{
  OSimple::draw(d);
  flame_.draw(d);
}


}

