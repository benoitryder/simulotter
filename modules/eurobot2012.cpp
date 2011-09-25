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


}

