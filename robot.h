#ifndef ROBOT_H
#define ROBOT_H

///@file

#include <cmath>
#include <vector>
#include "object.h"



/** @brief Robot base class
 */
class Robot: public Object
{
public:
  Robot();
};


/** @brief Basic robot
 *
 * A robot with a simple asserv and a basic shape.
 * It is intended to be used as a base class for robots at first design stage
 * since it provides basic move and order features.
 *
 * @note Asserv moves the robot by setting velocity at each step (using
 * set_v()) which may cause odd behaviors.
 *
 * @todo Add a setPosAbove() function.
 */
class RBasic: public Robot
{
protected:
  /** @brief Empty constructor, for derivative classes only
   *
   * Accessors does not check whether body is null or not.
   * Subclasses should create the body as soon as possible to prevent errors.
   */
  RBasic();
public:

  RBasic(btCollisionShape *shape, btScalar m);

  /** @brief Setup function
   *
   * This function may be called in subclasses constructors to initialize the
   * main body.
   *
   * @note Shape is not freed by destructor.
   */
  void setup(btCollisionShape *shape, btScalar m);

  ~RBasic();

  virtual void addToWorld(Physics *physics);
  virtual void removeFromWorld();

  /** @brief Set main color
   *
   * Color is used in default drawing function and may ignored by subclass
   * implementations.
   */
  void setColor(const Color4 &color) { color_ = color; }

  virtual void draw(Display *d);

  /** Draw a small direction cone above the robot
   *
   * Current OpenGL matrix is supposed to be on robot center.
   */
  void drawDirection();

  static const float DIRECTION_CONE_R; ///< Direction cone radius
  static const float DIRECTION_CONE_H; ///< Direction cone height


  virtual const btTransform &getTrans() const { return body_->getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform &tr) { body_->setCenterOfMassTransform(tr); }

  /** @brief Update position and velocity values
   *
   * Update internal values which will be returned by binding calls.
   * This method should be called after each simulation step and before asserv
   * step.
   */
  virtual void update();

  /** @brief Turn and move forward asserv
   *
   * Simple asserv: first the robot turns to face the target point. Then it
   * moves forward toward the target.
   */
  virtual void asserv();

  /** @name Basic methods used in strategy
   */
  //@{
  const btVector2 &get_xy()  const { return xy_; }
  btScalar get_a()  const { return a_; }
  btScalar get_v()  const { return v_;  }
  btScalar get_av() const { return av_; }

  void order_xy(btVector2 xy, bool rel=false);
  void order_a(btScalar a, bool rel=false);
  void order_xya(btVector2 xy, btScalar a, bool rel=false) { order_xy(xy,rel); order_a(a,rel); }
  void order_back(btScalar d);
  void order_stop() { order_ = ORDER_NONE; }

  bool is_waiting() { return order_ == ORDER_NONE; }
  //@}

  void set_v_max(btScalar v)  { v_max_  = v; }
  void set_av_max(btScalar v) { av_max_ = v; }

  void set_threshold_xy(btScalar t) { threshold_xy_ = t; }
  void set_threshold_a(btScalar t)  { threshold_a_  = t; }

protected:
  btRigidBody *body_;

  /// Robot main color
  Color4 color_;

  /** @name Position and velocity values
   *
   * These values are updated after calling \e update().
   */
  //@{
  btVector2 xy_; ///< Position
  btScalar  a_;  ///< Angular position
  btScalar  v_;  ///< Velocity
  btScalar  av_; ///< Angular velocity (radians)
  //@}

  btScalar v_max_;
  btScalar av_max_;

  /** @name Order targets
   */
  //@{
  btVector2 target_xy_;
  btScalar  target_a_;
  btVector2 target_back_xy_;
  //@}

  /** @brief Order types
   *
   * Orders are given as a bitset.
   *
   * Go back order is priority but does not cancel other orders.
   */
  enum
  {
    ORDER_NONE    =  0x0,
    ORDER_GO_XY   =  0x1,  // Position order
    ORDER_GO_A    =  0x2,  // Angle order
    ORDER_GO_BACK =  0x4,  // Go back
  };


  unsigned int order_;

  void set_v(btScalar v);
  void set_av(btScalar v);

  /// Asserv position threshold
  btScalar threshold_xy_;
  /// Asserv angle threshold
  btScalar threshold_a_;
};

#endif

