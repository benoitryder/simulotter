#ifndef OBJECT_H
#define OBJECT_H


#include <vector>
#include "global.h"


///@file


/** @brief Basic object
 *
 * Objects have to be added to the world (after initialization) using
 * Physics::addObject().
 */
class Object: public btRigidBody
{
public:
  /** @brief Default empty constructor
   *
   * Bullet does not provide empty constructor. Empty construction info are
   * provided and various initialization functions can be used to set values
   * afterwards.
   */
  Object();

  virtual ~Object() {}

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

  bool isInitialized() { return getCollisionShape() != NULL; }

  /// Draw the whole object
  virtual void draw();

  /** @brief Get object position
   * @note Identical to getCenterOfMassPosition()
   */
  const btVector3 &getPos() const { return m_worldTransform.getOrigin(); }
  /** @brief Get object rotation
   * @note Identical to getCenterOfMassOrientation()
   */
  const btQuaternion getRot() const { return m_worldTransform.getRotation(); }

  /** @brief Set object position
   * @todo Change <tt>m_interpolation*</tt> too?
   */
  void setPos(const btVector3 &pos) { m_worldTransform.setOrigin(pos); }
  /** @brief Place above (not on or in) the ground
   * @sa Config::drop_epsilon
   */
  void setPos(const btVector2 &pos);
  /** @brief Set object rotation
   * @todo Change <tt>m_interpolation*</tt> too?
   */
  void setRot(const btQuaternion &rot) { m_worldTransform.setRotation(rot); }

protected:
  /// Change the GL matrix according to position and rotation
  static void drawTransform(const btTransform &transform);

  /// drawTransform(), instance version
  void drawTransform() { drawTransform(m_worldTransform); }

  /** @brief Draw a collision shape
   *
   * @note This function is based on <em>GL_ShapeDrawer::drawOpenGL</em> method
   * from <em>Bullet</em>'s demos.
   */
  static void drawShape(const btTransform &transform, const btCollisionShape *shape);
};


/// Colored object
class ObjectColor: public Object
{
public:
  ObjectColor(): Object() {}

  void setColor(Color4 color) { COLOR_COPY(this->color, color); }
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
  ~OGround();

  virtual void draw();

protected:
  /// Starting zone size (scaled)
  static const btScalar size_start;

  Color4 color;
  Color4 color_t1;
  Color4 color_t2;

private:
  static btBoxShape shape;
};


#endif
