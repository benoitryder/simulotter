#ifndef EUROBOT2012_H_
#define EUROBOT2012_H_

/** @file
 * @brief Implementation of Eurobot 2012 rules, Treasure Island
 */

#include "object.h"

namespace eurobot2012 {


class OGround2012: public OGround
{
 public:
  static const btVector2 SIZE;
  static const btScalar START_SIZE;

  OGround2012();
  ~OGround2012() {}

  virtual void draw(Display* d) const;
};


class OBullion: public OSimple
{
 public:
  static const btVector3 SIZE;  ///< Bounding box size
  static const btScalar MASS;
  static const btScalar A_SLOPE;

  OBullion();
  virtual void draw(Display* d) const;

 private:
  static SmartPtr<btConvexHullShape> shape_;
};


class OCoin: public OSimple
{
 public:
  static const btScalar DISC_HEIGHT;
  static const btScalar RADIUS;
  static const btScalar INNER_RADIUS;
  static const btScalar CUBE_SIZE;
  static const btScalar CUBE_OFFSET;
  static const btScalar MASS;

  OCoin(bool white);
  virtual void draw(Display* d) const;

 private:
  static SmartPtr<btCompoundShape> shape_;
  static btCylinderShapeZ shape_disc_;
  static btBoxShape shape_cube_;
};


}

#endif
