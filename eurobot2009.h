#ifndef EUROBOT2009_H
#define EUROBOT2009_H

#include "global.h"
#include "galipeur.h"

/** @file
 * @brief Implementation of Eurobot 2009 rules, Atlantis
 */


namespace eurobot2009
{
  /// Column element
  class OColElem: public OSimple
  {
  public:
    OColElem();
  private:
    static SmartPtr<btCylinderShapeZ> shape;
    static GLuint dl_id_static;
  };

  /// Lintels
  class OLintel: public OSimple
  {
  public:
    OLintel();
  private:
    static SmartPtr<btBoxShape> shape;
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

    void draw();

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
    static const btScalar radius;
    static const btScalar height;
  private:
    static SmartPtr<btCylinderShapeZ> shape;
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
    static SmartPtr<btCompoundShape> shape;
    static btBoxShape arm_shape;
    static btBoxShape back_shape;
    static btBoxShape bottom_shape;
  };


  /** @brief Rob'Otter robot
   *
   * Galipeur with a so called <em>pàchev</em> (<em>pince à cheveux</em>, means
   * <em>hair grip</em>) to carry column elements.
   */
  class Galipeur2009: public Galipeur
  {
    friend class LuaGalipeur2009;
  public:
    Galipeur2009(btScalar m);
    virtual ~Galipeur2009();

    virtual void addToWorld(Physics *physics);

    /** @brief Draw the robot
     *
     * Assumes that the robot is not rotated (robot's Z axis aligned with
     * world's Z axis).
     */
    void draw();

    virtual void setTrans(const btTransform &tr);

    virtual void do_asserv();

    /** @name Strategy functions and orders
     */
    //@{
    btScalar get_pachev_pos() const { return pachev_link->getLinearPos(); }

    void order_pachev_move(btScalar h);
    void order_pachev_release();
    void order_pachev_grab();
    void order_pachev_eject();

    btScalar target_pachev_pos;

    enum {
      ORDER_PACHEV_MOVE = 0x100,
    };

    void set_pachev_v(btScalar v)  { this->pachev_v  = v; }
    void set_pachev_eject_v(btScalar f)  { this->pachev_eject_v = f; }
    void set_threshold_pachev(btScalar t) { this->threshold_pachev = t; }

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
       * Checks assume that the dispenser is not rotated (pàchev axis aligned
       * with Z axis).
       */
      virtual bool checkCollideWithOverride(btCollisionObject *co);

      static const btScalar width;
      static const btScalar height;
      static const btScalar z_max; ///< Maximum Z position
    protected:
      /// Reset pàchev's transform according to robot's transform
      void resetTrans();
    private:
      static btBoxShape shape;
      Galipeur2009 *robot;
    };

    Pachev *pachev;
    btSliderConstraint *pachev_link;

    /// Pachev states
    typedef enum
    {
      PACHEV_RELEASE,
      PACHEV_GRAB,
      PACHEV_EJECT
    } PachevState;

    PachevState pachev_state;

    /// Pàchev slider velocity
    btScalar pachev_v;
    /// Velocity of ejected object
    btScalar pachev_eject_v;
    /// Asserv pàchev threshold
    btScalar threshold_pachev;
  };
}


#endif
