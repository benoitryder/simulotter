#ifndef OBJECT_H
#define OBJECT_H


#include <vector>
#include "global.h"

class Physics;


///@file


/** @brief Object abstract class
 */
class Object
{
protected:
  Object() {}
public:
  virtual ~Object() {}

  /** @brief Add an object in a physics world.
   *
   * Add bodies and constraint to the Bullet world.
   * Object is added to the physics object array, an exception is raised if it
   * was already in the array.
   *
   * @note Overload functions should call this parent function.
   */
  virtual void addToWorld(Physics *physics);

  /// Draw the whole object
  virtual void draw() = 0;

  /** @name Transformation, position and rotation accessors
   */
  //@{
  virtual const btTransform &getTrans() const = 0;
  virtual void setTrans(const btTransform &tr) = 0;
  const btVector3   &getPos() const { return getTrans().getOrigin(); }
  const btMatrix3x3 &getRot() const { return getTrans().getBasis();  }
  void setPos(const btVector3 &pos)   { setTrans(btTransform(getRot(), pos)); }
  void setRot(const btMatrix3x3 &rot) { setTrans(btTransform(rot, getPos())); }
  //@}

protected:
  /// Change the GL matrix according to position and rotation
  static void drawTransform(const btTransform &transform);

  /** @brief Draw a collision shape
   *
   * @note This function is based on <em>GL_ShapeDrawer::drawOpenGL</em> method
   * from <em>Bullet</em>'s demos.
   */
  static void drawShape(const btTransform &transform, const btCollisionShape *shape);
};


/** @brief Simple object
 *
 * Simple objects are one-part common colored objects.
 * A simple object directly inherit from btRigidBody.
 *
 * Initialization functions are provided to configure an object after its
 * creation.
 */
class OSimple: public Object, public btRigidBody
{
public:
  /** @brief Default empty constructor
   *
   * Bullet does not provide empty constructor. Empty construction info are
   * provided and various initialization functions can be used to set values
   * afterwards.
   */
  OSimple();
  OSimple(const btRigidBodyConstructionInfo &info): btRigidBody(info) {}

  virtual ~OSimple() {}

  /** @brief Set object shape
   *
   * In order to prevent LUA scripts to reassign a shape in subclasses, this
   * function will fail if shape is already set. Implementations may call bullet
   * functions if they really need to reassign shape.
   *
   * @note This function must be called to initialize the object.
   */
  void setShape(btCollisionShape *shape);

  /** @brief Set object mass and inertia
   *
   * Inertia is determined from mass and object shape.
   *
   * @note Object shape must be set before calling this function.
   */
  void setMass(btScalar mass);

  /** @brief Add an object in a physics world.
   * Object has to be initialized before being added to a world.
   */
  virtual void addToWorld(Physics *physics);

  bool isInitialized() { return getCollisionShape() != NULL; }

  /** @brief Set main color
   * Object color is used in default drawing function and may ignored by
   * subclass implementations.
   */
  void setColor(const Color4 &color) { this->color = color; }

  /// Draw the whole object
  virtual void draw();

  virtual const btTransform &getTrans() const { return getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform &tr) { setCenterOfMassTransform(tr); }

  /** @brief Place above (not on or in) the ground
   * @sa Config::drop_epsilon
   *
   * @note The setPos() name has not been reused because it would make the
   * compiler use the btVector2 version by default, even when passing a
   * btVector3 (since it can be converted to). This would lead to tricky bugs
   * if one forgets to force using Object::setPos().
   */
  void setPosAbove(const btVector2 &pos);

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
class OGround: public OSimple
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
  OGround(const Color4 &color, const Color4 &color_t1, const Color4 &color_t2);
  ~OGround();

  virtual void draw();

protected:
  /// Starting zone size (scaled)
  static const btScalar size_start;

  Color4 color_t1;
  Color4 color_t2;

private:
  static btBoxShape shape;
};


#endif
