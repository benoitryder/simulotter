#ifndef OBJECT_H
#define OBJECT_H

#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <ode/ode.h>
#include "lua_utils.h"

#include "colors.h"

///@file


/// Convenient macro to go execute instructions on all body's geoms.
#define OBJECT_FOREACH_GEOM(body,g,instructions) \
  for( dGeomID g=dBodyGetFirstGeom(body); g!=NULL; g=dBodyGetNextGeom(g) ) \
    {instructions;}


/** @brief Basic object
 *
 * New objects are automatically added to the simulation objects and geoms are
 * put in the global space.
 *
 * There a two object types: static and dynamic. Static objects are dynamic
 * objects without a physical body, they collide but are not simulated.
 *
 * Static object bodies are put in the dummy worl \e world_void.
 */
class Object
{
protected:
  /** @brief Empty constructor
   *
   * Only for subclasses which have to initialize complexe geom structures
   * before calling the parent constructor. They can use ctor_init() or
   * ctor_mass() instead.
   */
  Object();

  /// Common constructor initializations
  void ctor_init(dGeomID *geoms, int nb, dBodyID body);

  /// Common mass initializations
  void ctor_init(dGeomID *geoms, int nb, dReal m);

public:
  /** @brief Create an object with multiple geoms
   *
   * If \e body is null, a static object is created.
   *
   * Each geom offset is set to its position (before begin attached to the body).
   */
  Object(dGeomID *geoms, int nb, dBodyID body=NULL);
  /// Single geom constructor
  Object(dGeomID geom, dBodyID body=NULL);
  /** @brief Mass constructor
   *
   * Create an object with given geoms and mass.
   * For single geom object, the body shape is determined from the geom shape.
   * Otherwise the bounding box is used.
   *
   * @warning For single geom objects, if the geom has an offset the mass will
   * not be set correctly.
   */
  Object(dGeomID *geoms, int nb, dReal m);
  /// Mass constructor, single geom version
  Object(dGeomID geom, dReal m);


  virtual ~Object();

  /// Get bouding box
  void get_aabb(dReal aabb[6]);

  /** @name Category and collide bitfields operations
   *
   * All geoms in a same objects should have the same category and collide
   * bits. All operations set the same bitfield on each geoms.
   */
  //@{
  unsigned long get_category() const;
  unsigned long get_collide()  const;
  void set_category(unsigned long cat);
  void set_collide (unsigned long col);
  void add_category(unsigned long cat) { set_category(get_category()|cat); }
  void add_collide (unsigned long col) { set_collide(get_collide()|col);   }
  void sub_category(unsigned long cat) { set_category(get_category()&~cat); }
  void sub_collide (unsigned long col) { set_collide(get_collide()&~col);   }
  //@}

  bool is_visible() const { return this->visible; }

  const dReal *get_pos() const { return dBodyGetPosition(body); }
  void set_pos(dReal x, dReal y, dReal z) { dBodySetPosition(body, x, y, z); }
  /** @brief Place above (not on or in) the ground
   * @sa Config::drop_epsilon
   */
  void set_pos(dReal x, dReal y);

  void set_rot(const dQuaternion q) { dBodySetQuaternion(body, q); }

  void set_visible(bool b) { this->visible = b; }

  /// Draw the whole object
  virtual void draw();


protected:
  dBodyID body;

  /// Another world where nothing happens, for static object bodies
  static dWorldID world_void;

  /// Object is not drawn if not visible
  bool visible;

  /// Change the GL matrix according to position and rotation
  static void draw_move(dGeomID geom);
  /// Change the GL matrix according to position and rotation, body version
  void draw_move();

  /// Draw a given geometry
  static void draw_geom(dGeomID geom);
};


/// Colored object
class ObjectColor: public Object
{
protected:
  ObjectColor(): Object() {}

public:
  ObjectColor(dGeomID *geoms, int nb=1, dBodyID body=NULL):
    Object(geoms, nb, body) {}
  ObjectColor(dGeomID geom, dBodyID body=NULL):
    Object(geom, body) {}
  ObjectColor(dGeomID *geoms, int nb, dReal m):
    Object(geoms, nb, m) {}
  ObjectColor(dGeomID geom, dReal m):
    Object(geom, m) {}

  void set_color(Color4 color) { COLOR_COPY(this->color, color); }
  virtual void draw() { glColor3fv(color); Object::draw(); }

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
