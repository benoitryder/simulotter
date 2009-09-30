#ifndef EUROBOT2010_H
#define EUROBOT2010_H

#include "global.h"
#include "galipeur.h"

/** @file
 * @brief Implementation of Eurobot 2010 rules, Feed the World
 */


namespace eurobot2010
{
  /** @brief Raised zone
   *
   * Object center Z position is 0.
   */
  class ORaisedZone: public OSimple
  {
  public:
    static const btScalar width;
    static const btScalar height;
    static const btScalar bottom_length;
    static const btScalar top_length;
    static const btScalar strip_length;
    static const btScalar wall_width;
    static const btScalar wall_height;
    static const btScalar wall_bottom_length;
    static const btScalar wall_top_length;

    ORaisedZone();
    virtual void draw();
  protected:
    void draw_wall();
  private:
    static SmartPtr<btCompoundShape> shape;
    static btConvexHullShape body_shape;
    static btConvexHullShape wall_shape;
  };

  /// Ear of corn
  class OCorn: public OSimple
  {
  public:
    OCorn();
  private:
    static SmartPtr<btCylinderShapeZ> shape;
  };

  /// Tomato
  class OTomato: public OSimple
  {
  public:
    OTomato();
  private:
    static SmartPtr<btSphereShape> shape;
  };

  /// Orange
  class OOrange: public OSimple
  {
  public:
    OOrange();
  private:
    static SmartPtr<btSphereShape> shape;
  };


  class LuaEurobotModule: public LuaModule
  {
  public:
    virtual void do_import(lua_State *L);
  };
}


#endif
