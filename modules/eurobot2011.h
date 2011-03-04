#ifndef EUROBOT2011_H_
#define EUROBOT2011_H_

/** @file
 * @brief Implementation of Eurobot 2011 rules, Chess'Up!
 */

#include "object.h"
#include "galipeur.h"


namespace eurobot2011
{

class OGround2011: public OGround
{
 public:
  static const btScalar SQUARE_SIZE;
  static const btScalar START_SIZE;

  OGround2011();
  ~OGround2011() {}

  virtual void draw(Display *d) const;
};


/** @brief Rob'Oter robot.
 *
 * Galipeur with mobile arms to catch pawns.
 */
class Galipeur2011: public Galipeur
{
 public:
  Galipeur2011(btScalar m);
  virtual ~Galipeur2011();

  virtual void addToWorld(Physics *physics);
  virtual void removeFromWorld();

  virtual void draw(Display *d) const;

  virtual void setTrans(const btTransform &tr);

  friend class PawnArm;
  class PawnArm: public btRigidBody
  {
    friend class Galipeur2011;
   public:
    static const btScalar RADIUS;
    static const btScalar LENGTH;
    static const btScalar MASS;

    void draw(Display *d) const;

   protected:
    PawnArm(Galipeur2011 *robot, const btTransform &tr);
    ~PawnArm();
    /// Reset arm's transform according to robot's transform
    void resetTrans();

   private:
    static btCapsuleShape shape_;
    Galipeur2011 *robot_;
    btTransform robot_tr_; ///< Transformation relative to the robot.
    btSliderConstraint *robot_link_;
  };

 private:
  PawnArm *arms_[2];
};

}


#endif
