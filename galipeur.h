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
  friend class LuaGalipeur;
public:
  Galipeur(btScalar m);
  virtual ~Galipeur();

  virtual void addToWorld(Physics *physics);
  virtual void removeFromWorld();

  void setColor(const Color4 &color) { color_ = color; }

  /** @brief Draw the robot
   *
   * Assumes that the robot is not rotated (robot's Z axis aligned with world's
   * Z axis).
   */
  virtual void draw();

  virtual const btTransform &getTrans() const { return body_->getCenterOfMassTransform(); }
  virtual void setTrans(const btTransform &tr) { body_->setCenterOfMassTransform(tr); }

  /** @brief Asserv step
   *
   * Go in position and/or turn according to current target.
   */
  virtual void asserv();

  /** @name Strategy functions and orders
   */
  //@{
  typedef std::vector<btVector2> CheckPoints;

  const btVector2 get_xy() const { return this->getPos(); }
  const btVector2 get_v()  const { return body_->getLinearVelocity(); }
  btScalar get_a() const
  {
    btScalar a, p, r;
    this->getRot().getEulerYPR(a,p,r);
    return a;
  }
  btScalar get_av() const { return body_->getAngularVelocity().getZ(); }

  void order_xy(btVector2 xy, bool rel=false);
  void order_a(btScalar a, bool rel=false);
  void order_xya(btVector2 xy, btScalar a, bool rel=false);
  void order_stop();

  void order_trajectory(const CheckPoints &pts);

  bool stopped() const { return checkpoints_.empty(); }
  /// Return \e true if position target has been reached.
  inline bool order_xy_done() const;
  /// Return \e true if angle target has been reached.
  inline bool order_a_done() const;
  inline bool is_waiting() const { return order_xy_done() && order_a_done(); }
  /// Return the current checkpoint zero-based index.
  inline size_t current_checkpoint() const { return ckpt_ - checkpoints_.begin(); }

  btScalar test_sensor(unsigned int i) const;
  //@}

  /** @name Asserv configuration.
   */
  //@{
  void set_speed_xy(btScalar v, btScalar a) { ramp_xy_.var_v = v; ramp_xy_.var_acc = a; }
  void set_speed_a (btScalar v, btScalar a) { ramp_a_ .var_v = v; ramp_a_ .var_acc = ramp_a_ .var_dec = a; }
  void set_speed_steering(btScalar v, btScalar a) { v_steering_ = v; va_steering_ = a; }
  void set_speed_stop(btScalar v, btScalar a) { v_stop_ = v; va_stop_ = a; }
  void set_threshold_stop(btScalar r, btScalar l) { threshold_stop_ = r; threshold_a_ = l; }
  void set_threshold_steering(btScalar t) { threshold_steering_ = t; }
  //@}

  /** @brief Implementation of a quadramp filter.
   *
   * It is not an exact equivalent of the aversive module, it is only intended
   * to have the same behavior. Actually it offers more possibilities.
   *
   * Deceleration distance can be computed as following:
   * \f{eqnarray*}{
   *   v_{dec}(t) & = & (v-v_0) - a_{dec} t \\
   *   x_{dec}(t) & = & (v-v_0) t - \frac12 a_{dec} t^2 \\
   *   t_{dec}    & = & \frac{v-v_0}{a_{dec}} \\
   *   d_{dec}    & = & x_{dec}(t_{dec}) = \frac12 \frac{(v-v_0)^2}a_{dec}
   * \f}
   */
  class Quadramp
  {
  public:
    Quadramp(): var_v(0), var_v0(0), var_acc(0), var_dec(0), cur_v_(0) {}
    /// Reset current values.
    void reset(btScalar v=0) { cur_v_ = v; }
    /** @brief Feed the filter and return the new velocity.
     *
     * @param d   actual distance to target
     * @param dt  elapsed time since the last step
     */
    btScalar step(btScalar d, btScalar dt);

  public:
    btScalar var_v;    ///< maximum speed (cruise speed)
    btScalar var_v0;   ///< maximum speed when reaching target
    btScalar var_acc;  ///< maximum acceleration
    btScalar var_dec;  ///< maximum deceleration
  private:
    btScalar cur_v_;
  };

  /** @name Shape constants
   */
  //@{
  static const btScalar z_mass;   ///< Z position of the center of mass
  static const btScalar ground_clearance;
  static const btScalar angle_offset; ///< angle of the first wheel

  static const btScalar height;   ///< body height
  static const btScalar side;     ///< triangle side half size
  static const btScalar w_block;  ///< motor block half width (small side)
  static const btScalar r_wheel;  ///< wheel radius
  static const btScalar h_wheel;  ///< wheel height (when laid flat)

  static const btScalar d_side;   ///< distance center/triangle side
  static const btScalar d_wheel;  ///< distance center/wheel side
  static const btScalar a_side;   ///< center angle of triangle side
  static const btScalar a_wheel;  ///< center angle of wheel side
  static const btScalar radius;   ///< outer circle radius
  //@}

protected:
  btRigidBody *body_;

  Color4 color_;

  /// Sharp positions
  std::vector<btTransform> sharps_trans_;

  /** @name Orders and parameters.
   */
  //@{

  CheckPoints checkpoints_; ///< checkpoint list, empty if asserv is not running
  CheckPoints::iterator ckpt_; ///< current checkpoint, never at end
  Quadramp ramp_xy_;
  btScalar v_steering_, va_steering_, threshold_steering_;
  btScalar v_stop_, va_stop_, threshold_stop_;

  btScalar target_a_;
  Quadramp ramp_a_;
  btScalar ramp_last_t_; ///< Last update time of ramps.
  btScalar threshold_a_;

  //@}

  void set_v(btVector2 vxy);
  void set_av(btScalar v);
  bool lastCheckpoint() const { return ckpt_ >= checkpoints_.end()-1; }

private:
  static SmartPtr<btCompoundShape> shape_;
  static btConvexHullShape body_shape_;
  static btBoxShape wheel_shape_;
  /** @brief Display list shared by all instances
   * @todo Created display list is not deleted.
   */
  static GLuint dl_id_static_;
};


bool Galipeur::order_xy_done() const
{
  if( this->stopped() )
    return true;
  if( !this->lastCheckpoint() )
    return false;
  return ((*ckpt_) - get_xy()).length() < threshold_stop_;
}

bool Galipeur::order_a_done() const
{
  if( this->stopped() )
    return true;
  return btFabs(btNormalizeAngle( target_a_-get_a() )) < threshold_a_;
}


#endif
