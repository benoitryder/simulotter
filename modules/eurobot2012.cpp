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

    const Color4 color_sand(0xfc,0xbd,0x1f); // RAL 1023
    const Color4 color_jungle(0x4f,0xa8,0x33); // RAL 6018

    // map island
    btglTranslate(0, 0.5*size_.y(), 0);
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

