#ifndef ROBOT_H
#define ROBOT_H

#include <SDL/SDL.h>
#include <math.h>
#include <vector>
#include "global.h"


///@file


/** @brief Robot base class
 */
class Robot: public Object
{
  friend class LuaRobot;
public:

  Robot();
  virtual ~Robot();

protected:
  /// Lua instance reference
  int ref_obj;

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
 * @todo Add a setPosStart() function.
 */
class RBasic: public Robot
{
  friend class LuaRBasic;
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

  /** @brief Set main color
   *
   * Color is used in default drawing function and may ignored by subclass
   * implementations.
   */
  void setColor(const Color4 &color) { this->color = color; }

  virtual void draw();

  /** Draw a small direction cone above the robot
   *
   * Current OpenGL matrix si supposed to be on robot center.
   */
  void drawDirection();

  virtual const btTransform &getTrans() const { return body->getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform &tr) { body->setCenterOfMassTransform(tr); }

  /** @brief Update position and velocity values
   *
   * Update internal values which will be returned by lua calls.
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
  const btVector2 &get_xy()  const { return this->xy; }
  btScalar get_a()  const { return this->a; }
  btScalar get_v()  const { return this->v;  }
  btScalar get_av() const { return this->av; }

  void order_xy(btVector2 xy, bool rel=false);
  void order_a(btScalar a, bool rel=false);
  void order_xya(btVector2 xy, btScalar a, bool rel=false) { order_xy(xy,rel); order_a(a,rel); }
  void order_back(btScalar d);
  void order_stop() { order = ORDER_NONE; }

  bool is_waiting() { return order == ORDER_NONE; }
  //@}

  void set_v_max(btScalar v)  { this->v_max  = v; }
  void set_av_max(btScalar v) { this->av_max = v; }

  void set_threshold_xy(btScalar t) { this->threshold_xy = t; }
  void set_threshold_a(btScalar t)  { this->threshold_a  = t; }

protected:
  btRigidBody *body;

  /// Robot main color
  Color4 color;

  /** @name Position and velocity values
   *
   * These values are updated after calling \e update().
   */
  //@{
  btVector2 xy; ///< Position
  btScalar  a;  ///< Angular position
  btScalar  v;  ///< Velocity
  btScalar  av; ///< Angular velocity (radians)
  //@}

  btScalar v_max;
  btScalar av_max;

  /** @name Order targets
   */
  //@{
  btVector2 target_xy;
  btScalar  target_a;
  btVector2 target_back_xy;
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


  unsigned int order;

  void set_v(btScalar v);
  void set_av(btScalar v);

  /// Asserv position threshold
  btScalar threshold_xy;
  /// Asserv angle threshold
  btScalar threshold_a;
};

#endif

