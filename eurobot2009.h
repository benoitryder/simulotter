#ifndef RULES2009_H
#define RULES2009_H

#include "physics.h"
#include "object.h"

/** @file
 * @brief Implementation of Eurobot 2009 rules, Atlantis
 */


namespace eurobot2009
{
  /// Column element
  class OColElem: public ObjectColor
  {
  public:
    OColElem()
    {
      add_geom( dCreateCylinder(0, 0.035, 0.030) );
      set_mass( 0.100 );
      init();
      set_category(CAT_ELEMENT);
    }
  };

  /// Lintels
  class OLintel: public ObjectColor
  {
  public:
    OLintel()
    {
      add_geom( dCreateBox(0, 0.200, 0.070, 0.030) );
      set_mass( 0.300 );
      init();
      set_category(CAT_ELEMENT);
    }
  };


  /// Column dispense
  class ODispenser: public ObjectColor
  {
  public:
    ODispenser();

    /** @brief Set dispenser position from its attach point
     *
     * @param  x  attach x position
     * @param  y  attach y position
     * @param  z  space between the dispenser and the ground
     * @param  side dispenser attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void set_pos(dReal x, dReal y, dReal z, int side);

    /** @brief Put an object in the dispenser
     *
     * @param  o  Object to move in the dispenser
     * @param  z  Z position, in global coordinates
     */
    void fill(Object *o, dReal z);

    void draw();

    /** @brief Dispenser collision
     * Elements can only fall along the z axis. Other objects are ignored.
     */
    virtual bool collision_handler(Physics *physics, dGeomID o1, dGeomID o2);

  protected:
    static const dReal radius;
    static const dReal height;
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
    void set_pos(dReal d, int side);

    /// Put a lintel in the storage
    void fill(OLintel *o);

  };
}


#endif
