#ifndef GALIPEUR_H
#define GALIPEUR_H

#include <vector>
#include "global.h"

///@file


/** @brief Rob'Otter robot, Galipeur
 *
 * A triangular holonomic robot.
 *
 * Galipeur shares code with RBasic but has its own (re)implementation for
 * better flexibility, since it may (will?) evolves in a different way.
 */
class Galipeur: public Robot
{
  friend class LuaGalipeur;
public:
  Galipeur(btScalar m);
  virtual ~Galipeur();

  virtual void addToWorld(Physics *physics);
  virtual void removeFromWorld();

  /** @brief Draw the robot
   *
   * Assumes that the robot is not rotated (robot's Z axis aligned with world's
   * Z axis).
   */
  virtual void draw();

  virtual const btTransform &getTrans() const { return body->getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform &tr) { body->setCenterOfMassTransform(tr); }

  /** @brief Asserv step
   *
   * Go in position and/or turn according to current target.
   */
  virtual void asserv();

  /** @name Strategy functions and orders
   */
  //@{
  const btVector2 get_xy() const { return this->getPos(); }
  const btVector2 get_v()  const { return body->getLinearVelocity(); }
  btScalar get_a() const
  {
    btScalar a, p, r;
    this->getRot().getEulerYPR(a,p,r);
    return a;
  }
  btScalar get_av() const { return body->getAngularVelocity().getZ(); }

  void order_xy(btVector2 xy, bool rel=false);
  void order_a(btScalar a, bool rel=false);
  void order_xya(btVector2 xy, btScalar a, bool rel=false) { order_xy(xy,rel); order_a(a,rel); }
  void order_stop() { order = ORDER_NONE; }

  bool is_waiting() { return order == ORDER_NONE; }

  btScalar test_sensor(unsigned int i);
  //@}

  void set_v_max(btScalar v)  { this->v_max  = v; }
  void set_av_max(btScalar v) { this->av_max = v; }

  void set_threshold_xy(btScalar t) { this->threshold_xy = t; }
  void set_threshold_a(btScalar t)  { this->threshold_a  = t; }

  /** @name Shape constants
   */
  //@{
  static const btScalar height;
  static const btScalar side;     ///< triangle side half size
  static const btScalar r_wheel;  ///< wheel radius
  static const btScalar h_wheel;  ///< wheel height (when laid flat)

  static const btScalar d_side;   ///< distance center/triangle side
  static const btScalar d_wheel;  ///< distance center/wheel side
  static const btScalar a_side;   ///< center angle of triangle side
  static const btScalar a_wheel;  ///< center angle of wheel side
  static const btScalar radius;   ///< outer circle radius
  //@}

protected:
  btRigidBody *body;

  /// Sharp positions
  std::vector<btTransform> sharps_trans;

  btScalar v_max;
  btScalar av_max;

  /** @name Order targets
   */
  //@{
  btVector2 target_xy;
  btScalar  target_a;
  //@}

  /** @brief Order types
   *
   * Orders are given as a bitset.
   */
  enum
  {
    ORDER_NONE    =  0x0,
    ORDER_GO_XY   =  0x1,  // Position order
    ORDER_GO_A    =  0x2,  // Angle order
  };

  unsigned int order;

  void set_v(btVector2 vxy);
  void set_av(btScalar v);

  /// Asserv position threshold
  btScalar threshold_xy;
  /// Asserv angle threshold
  btScalar threshold_a;

private:
  static SmartPtr<btCompoundShape> shape;
  static btConvexHullShape body_shape;
  static btBoxShape wheel_shape;
  /** @brief Display list shared by all instances
   * @todo Created display list is not deleted.
   */
  static GLuint dl_id_static;
};


#endif
