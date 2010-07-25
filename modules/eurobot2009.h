#ifndef EUROBOT2009_H_
#define EUROBOT2009_H_

/** @file
 * @brief Implementation of Eurobot 2009 rules, Atlantis
 */

#include "galipeur.h"


namespace eurobot2009
{
  /// Column element
  class OColElem: public OSimple
  {
  public:
    OColElem();
  private:
    static SmartPtr<btCylinderShapeZ> shape_;
  };

  /// Lintels
  class OLintel: public OSimple
  {
  public:
    OLintel();
  private:
    static SmartPtr<btBoxShape> shape_;
  };


  /// Column dispenser
  class ODispenser: public OSimple
  {
  public:
    ODispenser();

    /** @brief Set dispenser position from its attach point
     *
     * @param  v     attach point position, z is the space between the
     *               dispenser and the ground
     * @param  side  dispenser attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void setPos(const btVector3 &v, int side);

    /** @brief Put an object in the dispenser
     *
     * @param  o  Object to move in the dispenser
     * @param  z  Z position, in global coordinates
     */
    void fill(Object *o, btScalar z);

    void draw(Display *d);

    /** @brief Collision check to disable collision with stored items
     *
     * A stored item is an object whose position is near the dispenser axis,
     * whichever its z position.
     *
     * Check assumes that the dispenser is not rotated (dispenser axis aligned
     * with Z axis).
     */
    virtual bool checkCollideWithOverride(btCollisionObject *co);

  protected:
    static const btScalar RADIUS;
    static const btScalar HEIGHT;
  private:
    static SmartPtr<btCylinderShapeZ> shape_;
  };

  /// Lintel storage
  class OLintelStorage: public OSimple
  {
  public:
    OLintelStorage();

    /** @brief Set position from attach point
     *
     * Attach point is the lintel storage corner in contact with table wall.
     *
     * @param  d    attach position on the given wall
     * @param  side attach side (0 top, 1 right, 2 bottom, 3 left)
     */
    void setPos(btScalar d, int side);

    /// Put a lintel in the storage
    void fill(OLintel *o);

  private:
    static SmartPtr<btCompoundShape> shape_;
    static btBoxShape arm_shape_;
    static btBoxShape back_shape_;
    static btBoxShape bottom_shape_;
  };


  /** @brief Rob'Otter robot
   *
   * Galipeur with a so called <em>pàchev</em> (<em>pince à cheveux</em>, means
   * <em>hair grip</em>) to carry column elements.
   */
  class Galipeur2009: public Galipeur
  {
  public:
    Galipeur2009(btScalar m);
    virtual ~Galipeur2009();

    virtual void addToWorld(Physics *physics);
    virtual void removeFromWorld();

    /** @brief Draw the robot
     *
     * Assumes that the robot is not rotated (robot's Z axis aligned with
     * world's Z axis).
     */
    virtual void draw(Display *d);

    virtual void setTrans(const btTransform &tr);

    /** @brief Asserv step
     *
     * Move/turn the robot and the pàchev.
     */
    virtual void asserv();

    /** @name Strategy functions and orders
     */
    //@{
    btScalar get_pachev_pos() const { return pachev_link_->getLinearPos(); }

    void order_pachev_move(btScalar h);
    void order_pachev_release();
    void order_pachev_grab();
    void order_pachev_eject();

    btScalar target_pachev_pos_;

    void set_pachev_v(btScalar v)  { pachev_v_  = v; }
    void set_pachev_eject_v(btScalar f)  { pachev_eject_v_ = f; }
    void set_threshold_pachev(btScalar t) { threshold_pachev_ = t; }

    //@}

  private:
    friend class Pachev;
    /** @brief Pàchev body class
     *
     * A dedicated class is defined to override collision check.
     */
    class Pachev: public btRigidBody
    {
      friend class Galipeur2009;
    public:
      Pachev(Galipeur2009 *robot);
      ~Pachev();

      /** @brief Pàchev collision check
       *
       * Collision depends on pachev state:
       *  - relaxed: objects near the pàchev axis (whichever its z
       * position) does not collide;
       *  - grab: inside objects are constrained;
       *  - eject: special constraint.
       *
       * Checks assume that objects are not rotated (pàchev axis aligned
       * with Z axis).
       */
      virtual bool checkCollideWithOverride(btCollisionObject *co);

      static const btScalar WIDTH;
      static const btScalar HEIGHT;
      static const btScalar Z_MAX; ///< Maximum Z position
    protected:
      /// Reset pàchev's transform according to robot's transform
      void resetTrans();

    private:
      static btBoxShape shape_;
      Galipeur2009 *robot_;
    };

    Pachev *pachev_;
    btSliderConstraint *pachev_link_;

    /// Release constraints on caught objects.
    void releaseObjects();

    /// Pachev states
    enum PachevState
    {
      PACHEV_RELEASE,
      PACHEV_GRAB,
      PACHEV_EJECT
    };

    PachevState pachev_state_;
    bool pachev_moving_; ///< true if the pàchev moving order is active

    /// Pàchev slider velocity
    btScalar pachev_v_;
    /// Velocity of ejected object
    btScalar pachev_eject_v_;
    /// Asserv pàchev threshold
    btScalar threshold_pachev_;
  };

}


#endif
