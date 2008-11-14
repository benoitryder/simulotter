#ifndef OBJECT_H
#define OBJECT_H

#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <ode/ode.h>
#include <vector>
#include "lua_utils.h"
#include "colors.h"

///@file


/** @brief Basic object
 *
 * New objects are automatically added to the simulation objects and geoms are
 * put in the global space.
 *
 * There a two object types: static and dynamic. Static objects are disabled and 
 * not updated during a simulation step.
 */
class Object
{
public:
  Object();

  /// Add a geom to the object
  void add_geom(dGeomID geom);

  /** @brief Set object body
   *
   * If \e body is null, this method has no effect.
   * Body cannot be set if mass has been set.
   *
   * @sa init()
   */
  void set_body(dBodyID body);

  /** @brief Set object mass
   *
   * If \e m is negative (or null), this method has no effect.
   * Mass cannot be set if body has been set.
   *
   * @sa init()
   */
  void set_mass(dReal m);

  /** @brief Initialize the object
   *
   * This function must be called for each object when all object
   * elements have been set. Geoms and body of an initialized object
   * cannot be modified.
   *
   * Geoms are put in the global space and attached to the body.
   * Each geom offset is set to its position (before begin attached).
   * Object is put to the physics object vector.
   *
   * If a mass has been set, a body is created with mass attributes
   * determined from geoms.
   *
   * Category bits are cleared, collide bits are set to CAT_ALL.
   * If a body or a mass has been set, object is dynamic (CAT_DYNAMIC
   * category, does not collide static objects).
   *
   * @note A non initialized object cannot be moved or rotated.
   *
   * @note Subclasses may initialize the object in their constructor.
   */
  virtual void init();

  bool is_initialized() { return this->initialized; }

  virtual ~Object();

  /// Get bouding box
  void get_aabb(dReal aabb[6]);

  /** @name Category and collide bitfields operations
   *
   * All geoms in a same objects should have the same category and collide
   * bits. All operations set the same bitfield on each geoms.
   */
  //@{
  unsigned long get_category();
  unsigned long get_collide();
  void set_category(unsigned long cat);
  void set_collide (unsigned long col);
  void add_category(unsigned long cat) { set_category(get_category()|cat); }
  void add_collide (unsigned long col) { set_collide(get_collide()|col);   }
  void sub_category(unsigned long cat) { set_category(get_category()&~cat); }
  void sub_collide (unsigned long col) { set_collide(get_collide()&~col);   }
  //@}

  bool is_visible() const { return this->visible; }

  const dReal *get_pos() const { return dBodyGetPosition(body);   }
  const dReal *get_rot() const { return dBodyGetQuaternion(body); }

  void set_pos(dReal x, dReal y, dReal z);
  /** @brief Place above (not on or in) the ground
   * @sa Config::drop_epsilon
   */
  void set_pos(dReal x, dReal y);

  void set_rot(const dQuaternion q);

  void set_visible(bool b) { this->visible = b; }

  /// Draw the whole object
  virtual void draw();


protected:
  dBodyID body;
  std::vector<dGeomID> geoms;

  /// Change the GL matrix according to position and rotation
  static void draw_move(dGeomID geom);
  /// Change the GL matrix according to position and rotation, body version
  void draw_move();

  /// Draw a given geometry
  static void draw_geom(dGeomID geom);

private:
  /** @brief Indicate whether the object is initialized
   * @sa init()
   */
  bool initialized;

  /// Mass used for initialization
  dReal init_mass;

  /// Object is not drawn if not visible
  bool visible;
};


/// Colored object
class ObjectColor: public Object
{
public:
  ObjectColor() {}

  void set_color(Color4 color) { COLOR_COPY(this->color, color); }
  virtual void draw() { glColor4fv(color); Object::draw(); }

protected:
  /// Object main color
  Color4 color;
};


/** @brief Table ground
 *
 * Standard table ground with starting areas.
 *
 * Areas are positioned at positive y.
 * First team field part is at negative x.
 */
class OGround: public Object
{
public:

  /** @brief Constructor
   *
   * @param color     table color
   * @param color_t1  first team color
   * @param color_t2  second team color
   *
   * @note Only the first 3 color values are used.
   */
  OGround(const Color4 color, const Color4 color_t1, const Color4 color_t2);
  ~OGround() {}

  virtual void draw();

protected:
  static const dReal size_z = 0.1;

  /// Starting zone size
  static const dReal size_start = 0.5;

  /// Main box geom
  dGeomID geom_box;

  Color4 color;
  Color4 color_t1;
  Color4 color_t2;

};


#endif
