#ifndef OBJECT_H
#define OBJECT_H

#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <ode/ode.h>
#include "lua_utils.h"

#include "colors.h"

///@file


/** @brief Basic object
 *
 * A basic object collides but does not move (it is not simulated).
 * New objects are automatically added to the simulation objects and geoms are
 * put in the global space.
 *
 * @todo Allow multi-geom objects.
 * @todo Automatically push created geoms or create a dummy space for new
 * objects.
 */
class Object
{
protected:
  Object();

public:
  Object(dGeomID geom);
  virtual ~Object();

  unsigned long get_category() const { return dGeomGetCategoryBits(geom); }
  unsigned long get_collide()  const { return dGeomGetCollideBits(geom);  }
  void set_category(unsigned long cat) { dGeomSetCategoryBits(geom, cat); }
  void set_collide (unsigned long col) { dGeomSetCollideBits(geom, col);  }
  void add_category(unsigned long cat) { set_category(get_category()|cat); }
  void add_collide (unsigned long col) { set_collide(get_collide()|col);   }
  void sub_category(unsigned long cat) { set_category(get_category()&~cat); }
  void sub_collide (unsigned long col) { set_collide(get_collide()&~col);   }

  bool is_visible() const { return this->visible; }

  const dReal *get_pos() { return dGeomGetPosition(geom); }
  void set_pos(dReal x, dReal y, dReal z) { dGeomSetPosition(geom, x, y, z); }
  /// Place above (not on or in) the ground
  void set_pos(dReal x, dReal y);

  void set_rot(const dQuaternion q) { dGeomSetQuaternion(geom, q); }

  void set_visible(bool b) { this->visible = b; }

  /// Draw the whole object
  virtual void draw() { draw_geom(geom); }


protected:
  dGeomID geom;

  /// Object is not drawn if not visible
  bool visible;

  /** @brief Change the GL matrix according to position and rotation
   */
  static void draw_move(dGeomID geom);
  void draw_move() { draw_move(this->geom); }

  /// Draw a given geometry
  static void draw_geom(dGeomID geom);

};


/** @brief Moving object
 */
class ObjectDynamic: public Object
{
protected:
  ObjectDynamic();

public:
  /// Generic constructor
  ObjectDynamic(dGeomID geom, dBodyID body);

  /** @brief Convenient constructor
   *
   * Create an object with given geometry and mass attributes.
   * The body shape is determined from geometry shape. If it is
   * not a basic solid, the bounding box is used.
   */
  ObjectDynamic(dGeomID geom, dReal m);

  ~ObjectDynamic();

protected:

  dBodyID body;

  //TODO dGeomSetOffsetPosition
};



/// Colored static object
class ObjectColor: public Object
{
public:
  ObjectColor(dGeomID geom):
    Object(geom) {}

  void set_color(Color4 color) { COLOR_COPY(this->color, color); }
  virtual void draw() { glColor3fv(color); Object::draw(); }

protected:
  /// Object main color
  Color4 color;
};

/// Colored dynamic object
class ObjectDynamicColor: public ObjectDynamic
{
public:
  ObjectDynamicColor(dGeomID geom, dBodyID body):
    ObjectDynamic(geom, body) {};
  ObjectDynamicColor(dGeomID geom, dReal m):
    ObjectDynamic(geom, m) {};

  void set_color(Color4 color) { COLOR_COPY(this->color, color); }
  virtual void draw() { glColor3fv(color); Object::draw(); }

private:
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

  Color4 color;
  Color4 color_t1;
  Color4 color_t2;

};


#endif
