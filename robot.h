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

  unsigned int getTeam() const { return this->team; }

  /** @brief Register the robot in the match and set its team
   * @sa Match::registerRobot()
   */
  void matchRegister(unsigned int team=TEAM_INVALID);

  virtual void draw();

  /// Draw a small direction cone above the robot
  void drawDirection();

  /** @brief Init robot for the match.
   *
   * Get and cache update, asserv and strategy Lua functions, if any. This
   * method should be called before the match starts.
   */
  void matchInit();

  /** @brief Update asserv and strategy data
   *
   * Call the update Lua function (if any) or the do_update() method.
   * Robot instance is given as first argument.
   *
   * This function is called after each simulation step to update internal data
   * using simulation data (e.g. position).
   *
   * @todo Remove it (and use Bullet handlers instead)?
   */
  void update();

  /** @brief Asserv step
   *
   * Call the asserv Lua function (if any) or the do_asserv() method.
   * Robot instance is given as first argument.
   *
   * Search for a Lua asserv function and execute it.
   */
  void asserv();

  /** @brief Start or resume the strategy
   *
   * Call the strategy Lua function (if any) or the do_strategy() method.
   *
   * Non-Lua strategies are standard functions and explained in do_strategy().
   *
   * Lua strategies are coroutines, they continue their execution from the
   * point where they yielded. When the strategy coroutine finishes, robot
   * control stops: asserv and strategy are not called anymore.
   * Robot instance is given as first argument.
   *
   * @note Strategy is called after an asserv step.
   */
  void strategy();

protected:
  /// Default not implemented do_update() method
  virtual void do_update() { throw(Error("do_update() not implemented")); }
  /// Default not implemented do_asserv() method
  virtual void do_asserv() { throw(Error("do_asserv() not implemented")); }
  /** @brief Default not implemented do_strategy() method
   *
   * Strategy function is called with its last return value. If it returns -1,
   * it would not be called anymore.
   *
   * @param   val  last returned value, 0 at the first call
   * @return  The next parameter value or -1 to stop the robot.
   */
  virtual int do_strategy(int val) { throw(Error("do_strategy() not implemented")); }

private:

  /// Team number (TEAM_INVALID if not set)
  int team;

  /// Cached update function reference
  int ref_update;
  /// Cached asserv function reference
  int ref_asserv;
  /// Cached strategy function reference
  int ref_strategy;
  /// Strategy thread state
  lua_State *L_strategy;

protected:
  /// Lua instance reference
  int ref_obj;

};


/** @brief Basic robot
 *
 * A robot with a simple asserv.
 *
 * @note Asserv moves the robot by setting velocity at each step (using
 * set_v()) which may cause odd behaviors.
 */
class RBasic: public Robot
{
  friend class LuaRBasic;
public:
  RBasic();

  /** @brief Convenient constructor
   *
   * Create a default box robot with given size and mass.
   */
  RBasic(const btVector3 &halfExtents, btScalar m);

  ~RBasic();

  /** @brief Update position and velocity values
   */
  virtual void do_update();

  /** @brief Turn and move forward asserv
   *
   * Simple asserv: first the robot turns to face the target point. Then it
   * moves forward toward the target.
   */
  virtual void do_asserv();

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

private:
  btCollisionShape *shape;

protected:

  /** @name Position and velocity values
   *
   * These values are updated after calling \e update().
   */
  //@{
  btVector2 xy; ///< Position
  btScalar  a; ///< Position
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

