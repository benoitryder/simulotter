#ifndef EUROBOT2010_H_
#define EUROBOT2010_H_

/** @file
 * @brief Implementation of Eurobot 2010 rules, Feed the World
 */

#include "galipeur.h"
#include "lua_utils.h"


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

  /** @brief Ear of corn
   *
   * Ears of corn are \e planted in the ground with a plug making it pivoting
   * around the contact point with the ground.
   *
   * This pivot is simulated by a heavy and tiny rigid body, linked to the ear
   * of corn by a point-to-point constraint.
   * If the ear is hit with a sufficient force and begins to fall, the pivot
   * body and constraint are removed.
   */
  class OCorn: public OSimple
  {
  public:
    OCorn();

    /// Plant the corn in the ground.
    void plant(btScalar x, btScalar y);
    /// Uproot the corn, opposite of plant().
    void uproot();

    void removeFromWorld();
    /// Remove the ground attach when the corn falls.
    virtual void tickCallback();

  private:
    static SmartPtr<btCylinderShapeZ> shape;

    static const btScalar pivot_radius;
    static const btScalar pivot_mass;
    static SmartPtr<btCollisionShape> pivot_shape;

    /// Pivot rigid body.
    btRigidBody *opivot;
    /// Attach point with the pivot rigid body.
    btPoint2PointConstraint *pivot_attach;
  };

  /// Fake ear of corn
  class OCornFake: public OSimple
  {
  public:
    OCornFake();
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
