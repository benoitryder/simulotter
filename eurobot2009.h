#ifndef RULES2009_H
#define RULES2009_H

#include "global.h"

/** @file
 * @brief Implementation of Eurobot 2009 rules, Atlantis
 */


namespace eurobot2009
{
  /// Column element
  class OColElem: public ObjectColor
  {
  public:
    OColElem();
  private:
    static btCylinderShapeZ shape;
  };

  /// Lintels
  class OLintel: public ObjectColor
  {
  public:
    OLintel();
  private:
    static btBoxShape shape;
  };


  /// Column dispenser
  class ODispenser: public ObjectColor
  {
  public:
    ODispenser();

    /** @brief Set dispenser position from its attach point
     *
     * @param  v     attach point position, z is the space between the
     *               dispenser and the ground
     * @param  side  dispenser attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void setPos(const btVector3 &v, int side);

    /** @brief Put an object in the dispenser
     *
     * @param  o  Object to move in the dispenser
     * @param  z  Z position, in global coordinates
     */
    void fill(Object *o, btScalar z);

    void draw();

    /** @brief Collision check to disable collision with stored items
     *
     * A stored item is an object whose position is near the dispenser axis,
     * whichever its z position.
     *
     * Check assumes that the dispenser is not rotated (dispenser axis aligned
     * with Z axis).
     */
    virtual bool checkCollideWithOverride(btCollisionObject *co);

  protected:
    static const btScalar radius;
    static const btScalar height;
  private:
    static btCylinderShapeZ shape;
  };

  /// Lintel storage
  class OLintelStorage: public ObjectColor
  {
  public:
    OLintelStorage();

    /** @brief Set position from attach point
     *
     * Attach point is the lintel storage corner in contact with table wall.
     *
     * @param  d    attach position on the given wall
     * @param  side attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void setPos(btScalar d, int side);

    /// Put a lintel in the storage
    void fill(OLintel *o);

  private:
    static btCompoundShape shape;
    static btBoxShape arm_shape;
    static btBoxShape back_shape;
    static btBoxShape bottom_shape;
  };


  /** @brief Rob'Otter robot
   *
   * Triangular holonomic robot with a so called <em>pàchev</em> (<em>pince à
   * cheveux</em>, means <em>hair grip</em>) to carry column elements.
   */
  class RORobot: public RBasic //XXX
  {
  friend class LuaRORobot;
  public:
    RORobot(btScalar m);
    virtual ~RORobot();

    virtual void addToWorld(Physics *physics);

    /** @brief Draw the robot
     *
     * Assumes that the dispenser is not rotated (robot's Z axis aligned with
     * world's Z axis).
     */
    void draw();

    virtual void do_update();

    static const btScalar height;
    static const btScalar side;     ///< triangle side half size
    static const btScalar r_wheel;  ///< wheel radius
    static const btScalar h_wheel;  ///< wheel height (when laid flat)
    static const btScalar w_pachev; ///< pàchev width
    static const btScalar h_pachev; ///< pàchev height

    static const btScalar d_side;   ///< distance center/triangle side
    static const btScalar d_wheel;  ///< distance center/wheel side
    static const btScalar a_side;   ///< center angle of triangle side
    static const btScalar a_wheel;  ///< center angle of wheel side
    static const btScalar radius;   ///< outer circle radius

  private:
    static btCompoundShape shape;
    static btConvexHullShape body_shape;
    static btBoxShape wheel_shape;
    static btBoxShape pachev_shape;

    btRigidBody *pachev;
    btSliderConstraint *pachev_link;
  };
}


#endif
