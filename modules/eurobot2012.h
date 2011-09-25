#ifndef EUROBOT2012_H_
#define EUROBOT2012_H_

/** @file
 * @brief Implementation of Eurobot 2012 rules, Treasure Island
 */

#include "object.h"

namespace eurobot2012
{

class OGround2012: public OGround
{
 public:
  static const btVector2 SIZE;
  static const btScalar START_SIZE;

  OGround2012();
  ~OGround2012() {}

  virtual void draw(Display *d) const;
};

}


#endif
