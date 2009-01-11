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


  /// Rob'Otter robot
  class RORobot: public RBasic //XXX
  {
  friend class LuaRORobot;
  public:
    RORobot(btScalar m);
    virtual ~RORobot();

    /** @brief Draw the robot
     *
     * Assumes that the dispenser is not rotated (robot's Z axis aligned with
     * world's Z axis).
     */
    void draw();

    /// Distance from the center to a corner
    static const btScalar radius;
    static const btScalar height;

  private:
    static btCompoundShape shape;
    static btConvexHullShape body_shape;
    static btBoxShape wheel_shape;
  };
}


#endif
