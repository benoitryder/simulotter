#ifndef EUROBOT2011_H_
#define EUROBOT2011_H_

/** @file
 * @brief Implementation of Eurobot 2011 rules, Chess'Up!
 */

#include "object.h"
#include "galipeur.h"

namespace eurobot2011 {


class OGround2011: public OGroundSquareStart
{
 public:
  static const btVector2 SIZE;
  static const btScalar SQUARE_SIZE;
  static const btScalar START_SIZE;

  OGround2011();
  virtual ~OGround2011() {}

 protected:
  virtual void drawDisplayList() const;
};


/** @brief Pawn magnet
 *
 * Physics provided to enable() is only used to store the physical world of
 * constraints in order to create or release them.
 * Enabling or disabling the magnet does not add or remove it from the world.
 */
class Magnet: public btRigidBody
{
 public:
  static const btScalar RADIUS;

  Magnet();
  ~Magnet();
  /// Return \e true object grabbing is enabled
  bool enabled() const { return physics_ != NULL; }
  /// Enable object grabbing
  void enable(Physics* ph);
  /// Release all objects, disable object grabbing
  void disable();

  virtual bool checkCollideWithOverride(btCollisionObject* co);
 protected:
  static btSphereShape shape_;
  Physics* physics_; ///< Physical world, \e NULL if disabled
};


/** @brief A pawn with magnets
 *
 * Each pawn has two magnets, one on each face.
 *
 * @note The actual class is defined in Python.
 */
class MagnetPawn: public OSimple
{
 public:
  static const btScalar RADIUS;
  static const btScalar HEIGHT;

  MagnetPawn(btCollisionShape* sh, btScalar mass=0);
  virtual ~MagnetPawn();
  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();
  virtual void setTrans(const btTransform& tr);
 private:
  Magnet magnets_[2];
  btGeneric6DofConstraint* magnet_links_[2];
};


#define GALIPEUR2011_ARM_NB  2

/** @brief Rob'Oter robot
 *
 * Galipeur with mobile arms to catch pawns.
 */
class Galipeur2011: public Galipeur
{
 public:
  Galipeur2011(btScalar m);
  virtual ~Galipeur2011();

  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();
  virtual void draw(Display* d) const;
  virtual void setTrans(const btTransform& tr);
  /// Handle arm moves
  virtual void asserv();

  void set_arm_av(btScalar av) { arm_av_ = av; }

  friend class PawnArm;
  class PawnArm: public btRigidBody
  {
    friend class Galipeur2011;
   public:
    static const btScalar RADIUS;
    static const btScalar LENGTH;
    static const btScalar MASS;
    static const btScalar ANGLE_MIN;
    static const btScalar ANGLE_MAX;

    /// Get angle position (0 is lower, Pi/2 is raised)
    btScalar angle() const { return M_PI_2-robot_link_->getAngularPos(); }
    /// Raise the arm
    void raise();
    /// Lower the arm
    void lower();
    /// Enable magnet
    void grab();
    /// Disable magnet
    void release();
    void asserv();

    void draw(Display* d) const;

   protected:
    PawnArm(Galipeur2011* robot, const btTransform& tr);
    ~PawnArm();
    /// Reset arm's transform according to robot's transform
    void resetTrans();

    void addToWorld();
    void removeFromWorld();

   private:
    static btCapsuleShape shape_;
    Galipeur2011* robot_;
    btTransform robot_tr_; ///< Transformation relative to the robot
    btSliderConstraint* robot_link_;
    Magnet magnet_;
    btGeneric6DofConstraint* magnet_link_;
  };

  PawnArm* const* getArms() const { return arms_; }

 private:
  PawnArm* arms_[GALIPEUR2011_ARM_NB];
  btScalar arm_av_;  ///< Arm angle velocity
};


}

#endif
