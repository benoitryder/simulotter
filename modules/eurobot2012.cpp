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

    d->endDisplayList();
  }

  glPopMatrix();
}


}

