#ifndef GALIPEUR_H_
#define GALIPEUR_H_

///@file

#include <vector>
#include "robot.h"


/** @brief Rob'Otter robot, Galipeur
 *
 * A triangular holonomic robot.
 *
 * Galipeur shares code with RBasic but has its own (re)implementation for
 * better flexibility, since it may (will?) evolves in a different way.
 */
class Galipeur: public Robot
{
 public:
  Galipeur(btScalar m);
  virtual ~Galipeur();

  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();

  Color4 getColor() const { return color_; }
  void setColor(const Color4& color) { color_ = color; }

  /** @brief Draw the robot
   *
   * Assumes that the robot is not rotated (robot's Z axis aligned with world's
   * Z axis).
   */
  virtual void draw(Display* d) const;

  virtual const btTransform getTrans() const { return body_->getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform& tr) { body_->setCenterOfMassTransform(tr); }

  /** @brief Place above (not on or in) the ground
   *
   * @note The setPos() name has not been reused because it would make the
   * compiler use the btVector2 version by default, even when passing a
   * btVector3 (since it can be converted to). This would lead to tricky bugs
   * if one forgets to force using Galipeur::setPos().
   */
  void setPosAbove(const btVector2& pos);

  /** @brief Asserv step
   *
   * Go in position and/or turn according to current target.
   */
  virtual void asserv();

  /** @name Strategy functions and orders
   */
  //@{
  typedef std::vector<btVector2> CheckPoints;

  btScalar getAngle() const
  {
    const btMatrix3x3 m = getRot();
    return -btAtan2(m[0][1], m[0][0]);
  }
  btVector2 getVelocity() const { return body_->getLinearVelocity(); }
  btScalar getAngularVelocity() const { return body_->getAngularVelocity().getZ(); }

  void order_xy(btVector2 xy, bool rel=false);
  void order_a(btScalar a, bool rel=false);
  void order_xya(btVector2 xy, btScalar a, bool rel=false);
  void order_stop();

  void order_trajectory(const CheckPoints& pts);

  /// Return \e true if position target has been reached
  inline bool order_xy_done() const;
  /// Return \e true if angle target has been reached
  inline bool order_a_done() const;
  inline bool is_waiting() const { return order_xy_done() && order_a_done(); }
  /// Return the current zero-based checkpoint index
  inline size_t current_checkpoint() const { return ckpt_ - checkpoints_.begin(); }
  //@}

  /** @name Asserv configuration
   */
  //@{
  void set_speed_xy(btScalar v, btScalar a) { ramp_xy_.var_v_ = v; ramp_xy_.var_acc_ = a; }
  void set_speed_a(btScalar v, btScalar a) { ramp_a_.var_v_ = v; ramp_a_.var_acc_ = ramp_a_.var_dec_ = a; }
  void set_speed_steering(btScalar v, btScalar a) { v_steering_ = v; va_steering_ = a; }
  void set_speed_stop(btScalar v, btScalar a) { v_stop_ = v; va_stop_ = a; }
  void set_threshold_stop(btScalar r, btScalar l) { threshold_stop_ = r; threshold_a_ = l; }
  void set_threshold_steering(btScalar t) { threshold_steering_ = t; }
  //@}

  /** @brief Implementation of a quadramp filter
   *
   * It is not an exact equivalent of the aversive module, it is only intended
   * to have the same behavior. Actually it offers more possibilities.
   *
   * Computations are made on positive (absolute) distances and speeds.
   * Velocity signs has to be handled by the user.
   *
   * Deceleration distance can be computed as following:
   * \f{eqnarray*}{
   *   v_{dec}(t) & = & (v_{cur}-v_0) - a_{dec} t \\
   *   x_{dec}(t) & = & (v_{cur}-v_0) t - \frac12 a_{dec} t^2 \\
   *   t_{dec}    & = & \frac{v_{cur}-v_0}{a_{dec}} \\
   *   d_{dec}    & = & x_{dec}(t_{dec}) = \frac12 \frac{v_{cur}^2-v_0^2}a_{dec}
   * \f}
   */
  class Quadramp
  {
   public:
    Quadramp(): var_v_(0), var_v0_(0), var_acc_(0), var_dec_(0), cur_v_(0) {}
    /// Reset current values
    void reset(btScalar v=0) { cur_v_ = v; }
    /** @brief Feed the filter and return the new velocity
     *
     * @param dt  elapsed time since the last step
     * @param d   actual distance to target
     */
    btScalar step(btScalar dt, btScalar d);

   public:
    btScalar var_v_;  ///< maximum speed (cruise speed)
    btScalar var_v0_;  ///< maximum speed when reaching target
    btScalar var_acc_;  ///< maximum acceleration
    btScalar var_dec_;  ///< maximum deceleration
   private:
    btScalar cur_v_;
  };

  /** @name Shape constants
   */
  //@{
  static const btScalar Z_MASS;   ///< Z position of the center of mass
  static const btScalar GROUND_CLEARANCE;
  static const btScalar ANGLE_OFFSET;  ///< angle of the first wheel

  static const btScalar HEIGHT;  ///< body height
  static const btScalar SIDE;  ///< triangle side half size
  static const btScalar W_BLOCK;  ///< motor block half width (small side)
  static const btScalar R_WHEEL;  ///< wheel radius
  static const btScalar H_WHEEL;  ///< wheel height (when laid flat)

  static const btScalar D_SIDE;  ///< distance center/triangle side
  static const btScalar D_WHEEL;  ///< distance center/wheel side
  static const btScalar A_SIDE;  ///< center angle of triangle side
  static const btScalar A_WHEEL;  ///< center angle of wheel side
  static const btScalar RADIUS;  ///< outer circle radius
  //@}

 protected:
  btRigidBody* body_;

  Color4 color_;

  /** @name Orders and parameters
   */
  //@{

  CheckPoints checkpoints_; ///< checkpoint list, empty if asserv is not running
  CheckPoints::iterator ckpt_; ///< current checkpoint, never at end
  Quadramp ramp_xy_;
  btScalar v_steering_, va_steering_, threshold_steering_;
  btScalar v_stop_, va_stop_, threshold_stop_;

  btScalar target_a_;
  Quadramp ramp_a_;
  btScalar ramp_last_t_; ///< Last update time of ramps
  btScalar threshold_a_;

  //@}

  void set_v(btVector2 vxy);
  void set_av(btScalar v);
  bool lastCheckpoint() const { return ckpt_ >= checkpoints_.end()-1; }

 private:
  static SmartPtr<btCompoundShape> shape_;
  static btConvexHullShape body_shape_;
  static btBoxShape wheel_shape_;
};


bool Galipeur::order_xy_done() const
{
  if(checkpoints_.empty()) {
    return true;
  }
  if(!lastCheckpoint()) {
    return false;
  }
  return ((*ckpt_) - btVector2(getPos())).length() < threshold_stop_;
}

bool Galipeur::order_a_done() const
{
  return btFabs(btNormalizeAngle( target_a_-getAngle() )) < threshold_a_;
}


#endif
