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


}

