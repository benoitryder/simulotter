#ifndef EUROBOT2011_H_
#define EUROBOT2011_H_

/** @file
 * @brief Implementation of Eurobot 2011 rules, Chess'Up!
 */

#include "object.h"


namespace eurobot2011
{

class OGround2011: public OGround
{
 public:
  static const btScalar CASE_SIZE;
  static const btScalar START_SIZE;

  OGround2011();
  ~OGround2011() {}

  virtual void draw(Display *d);
};


}


#endif
