#ifndef RULES2009_H
#define RULES2009_H

#include "rules.h"
#include "physics.h"
#include "object.h"

/** @file
 * @brief Implementation of Eurobot 2009 rules, Atlantis
 */


namespace rules2009
{
  class RAtlantis: public Rules
  {
  public:
    RAtlantis();
    virtual ~RAtlantis();

    /// Get the score of the given team
    virtual int get_score(int team);

    /// Initialize the match
    virtual void init(int fconf);

    /** @brief Build a field configuration value
     *
     * @param col   Column placement (1 to 10)
     * @param disp  Dispensers placement (1 to 2)
     */
    static int create_conf(int col, int disp) { return col | (disp<<4); }


  private:

    /** @brief Column placements
     *
     * Each placement gives column position.
     */
    static const int col_placements[10][3];

    static const dReal col_space_x;
    static const dReal col_space_y;
    static const dReal col_offset_x;
    static const dReal col_offset_y;

    static const dReal disp_offset_x;
    static const dReal disp_offset_y;
    static const dReal disp_offset_z;

    static const dReal wall_width;
    static const dReal wall_height;
  };


  /// Column elements
  class OColElem: public ObjectDynamicColor
  {
  public:
    OColElem():
      ObjectDynamicColor(dCreateCylinder(0, 0.035, 0.030), 0.100)
    {
      set_category(CAT_ELEMENT);
    }
  };

  /// Lintels
  class OLintel: public ObjectDynamicColor
  {
  public:
    OLintel():
      ObjectDynamicColor(dCreateBox(0, 0.200, 0.070, 0.030), 0.300)
    {
      set_category(CAT_ELEMENT);
    }
  };


  /// Column dispenser
  class ODispenser: public ObjectColor
  {
  public:
    ODispenser();

    /** @brief Set dispenser position from its attach point
     *
     * @param  x  attach x position
     * @param  y  attach x position
     * @param  z  space between the dispenser and the ground
     * @param  side dispenser attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void set_pos(dReal x, dReal y, dReal z, int side);

    /** @brief Put an object in the dispenser
     *
     * @param  o  Object to move in the dispenser
     * @param  z  Z position, in the global coordinates
     */
    void fill(ObjectDynamic *o, dReal z);

    void draw();
  };

  /// Lintel storage
  class OLintelStorage: public ObjectColor
  {
  public:
    OLintelStorage();
  };
}


#endif
