#ifndef OBJECT_H_
#define OBJECT_H_

///@file

#include "smart.h"
#include "colors.h"

class Physics;
class Display;


/** @brief Object abstract class
 */
class Object: public SmartObject
{
 protected:
  Object(): physics_(NULL) {}
 public:
  virtual ~Object() {}

  /** @brief Add an object to a physical world
   *
   * Add bodies and constraint to the Bullet world.
   * Object is added to the physics object array.
   *
   * @note Overload functions should call this parent function.
   */
  virtual void addToWorld(Physics* physics);

  Physics* getPhysics() const { return physics_; }

  /** @brief Remove an object from its physical world
   *
   * It is the opposite of addToWorld().
   * Object is removed from the physics object array, an exception is raised if
   * was not in a world array.
   * The \e physics_ member is resetted to \e NULL.
   *
   * @note Overload functions should call this parent function.
   */
  virtual void removeFromWorld();

  /** @brief Callback for when a simulation substep happens
   *
   * This method is intended to be defined by each subclasse and should not be called, except by Physics.
   *
   * The callback is disabled until enableTickCallback() is called.
   */
  virtual void tickCallback();

  /// Draw the whole object
  virtual void draw(Display* d) const = 0;
  /** @brief Draw last object parts
   *
   * This is used for transparent parts which have to be drawn last.
   */
  virtual void drawLast(Display* d) const {}

  /** @name Transformation, position and rotation accessors
   */
  //@{
  virtual const btTransform getTrans() const = 0;
  virtual void setTrans(const btTransform& tr) = 0;
  const btVector3 getPos() const { return getTrans().getOrigin(); }
  const btMatrix3x3 getRot() const { return getTrans().getBasis(); }
  void setPos(const btVector3& pos) { setTrans(btTransform(getRot(), pos)); }
  void setRot(const btMatrix3x3& rot) { setTrans(btTransform(rot, getPos())); }
  //@}

 protected:
  /// Change the GL matrix according to position and rotation
  static void drawTransform(const btTransform& transform);

  /** @brief Draw a collision shape
   *
   * @note This function is based on <em>GL_ShapeDrawer::drawOpenGL</em> method
   * from <em>Bullet</em>'s demos.
   */
  static void drawShape(const btCollisionShape* shape);

  /** @brief Physical world the object was added to
   *
   * Since the world keep a reference to its objects, it will not be deleted
   * before them. This means that a deleted object has been removed from its
   * world.
   */
  Physics* physics_;

  /// Enable the tick callback
  void enableTickCallback();
  /// Disable the tick callback
  void disableTickCallback();
};


/** @brief Simple object
 *
 * Simple objects are one-part common colored objects.
 * A simple object directly inherit from btRigidBody.
 *
 * Initialization functions are provided to configure an object after its
 * creation.
 *
 * If a displaylist is set on the instance it is used for drawing.
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
  OSimple(btCollisionShape* sh, btScalar mass=0);
  OSimple(const btRigidBodyConstructionInfo& info): btRigidBody(info) {}

  virtual ~OSimple();

  /** @brief Set object shape
   *
   * In order to prevent shape reassignement in subclasses, this function will
   * fail if shape is already set. Implementations may call bullet functions if
   * they really need to reassign shape.
   *
   * @note This function must be called to initialize the object.
   */
  void setShape(btCollisionShape* shape);

  /** @brief Set object mass and inertia
   *
   * Inertia is determined from mass and object shape.
   *
   * @note Object shape must be set before calling this function.
   */
  void setMass(btScalar mass);
  btScalar getMass() const { return getInvMass() == 0 ? 0 : 1./getInvMass(); }

  /** @brief Add an object in a physics world
   *
   * Object has to be initialized before being added to a world.
   */
  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();

  bool isInitialized() { return getCollisionShape() != NULL; }

  Color4 getColor() const { return color_; }
  /** @brief Set main color
   *
   * Object color is used in default drawing function and may ignored by
   * subclass implementations.
   */
  void setColor(const Color4& color) { color_ = color; }

  /// Draw the object, if not transparent
  virtual void draw(Display* d) const;
  /// Draw the object last, if transparent
  virtual void drawLast(Display* d) const;
  /** @brief Draw the object, whichever the color
   *
   * This method is called by draw() and drawLast().
   * Shape drawing is stored in a display list.
   */
  void drawObject(Display* d) const;

  virtual const btTransform getTrans() const { return getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform& tr) { setCenterOfMassTransform(tr); }

  /** @brief Place above (not on or in) the ground
   *
   * @note The setPos() name has not been reused because it would make the
   * compiler use the btVector2 version by default, even when passing a
   * btVector3 (since it can be converted to). This would lead to tricky bugs
   * if one forgets to force using Object::setPos().
   */
  void setPosAbove(const btVector2& pos);

 protected:
  /// Object main color
  Color4 color_;
};


/** @brief Table ground
 *
 * Standard table ground with starting areas.
 *
 * Areas are positioned at positive y.
 * First team field part is at negative x.
 *
 * Default starting area size is 50cm.
 */
class OGround: public OSimple
{
 public:
  /** @brief Constructor
   *
   * @param size      table size
   * @param color     table color
   * @param color_t1  first team color
   * @param color_t2  second team color
   */
  OGround(const btVector2& size, const Color4& color, const Color4& color_t1, const Color4& color_t2);
  virtual ~OGround();

  btVector2 getSize() const { return btVector2(size_); }
  btScalar getStartSize() const { return start_size_; }
  void setStartSize(const btScalar& size) { start_size_ = size; }

  virtual void draw(Display* d) const;

 protected:
  Color4 color_t1_;
  Color4 color_t2_;
  btVector3 size_;
  btScalar start_size_;

  /// Draw the ground base
  void drawBase() const;
  /// Draw starting areas
  void drawStartingAreas() const;

 private:
  SmartPtr<btBoxShape> shape_;
};


#endif
