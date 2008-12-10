#ifndef ROBOT_H
#define ROBOT_H

#include <SDL/SDL.h>
#include <ode/ode.h>
#include <math.h>
#include <vector>

class Robot;

#include "object.h"
#include "match.h"


///@file


/** @brief Robot base class
 */
class Robot: public Object
{
  friend class LuaRobot;
public:

  Robot();

  /** @brief Initialize the robot
   *
   * Call parent initialization function, set category.
   */
  virtual void init();

  ~Robot();

  unsigned int get_team() const { return this->team; }

  /** @brief Register the robot in the match and set its team
   * @sa Match::register_robot()
   */
  void match_register(unsigned int team=TEAM_INVALID);

  virtual void draw();

  /// Draw a small direction cone above the robot
  void draw_direction();

  /** @brief Init robot for the match.
   *
   * Get and cache update, asserv and strategy Lua functions, if any. This
   * method should be called before the match starts.
   */
  void match_init();

  /** @brief Update asserv and strategy data
   *
   * Call the update Lua function (if any) or the do_update() method.
   * Robot instance is given as first argument.
   *
   * This function is called after ODE step to update internal data using ODE
   * data (e.g. position).
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
   * @param   val  last returned value, -1 at the first call
   * @return  The next parameter value or -1 to stop the robot.
   */
  virtual int do_strategy(int val) { throw(Error("do_strategy() not implemented")); }

private:

  /// Team number (TEAM_INVALID if not set)
  unsigned int team;

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
  RBasic(dReal lx, dReal ly, dReal lz, dReal m);

  /** @brief Initialize the robot
   *
   * Call parent initialization function, add motors, reset
   * orders.
   */
  virtual void init();

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

  /** @name Methods used in strategy
   */
  //@{
  dReal get_x()  const { return this->x;  }
  dReal get_y()  const { return this->y;  }
  dReal get_a()  const { return this->a;  }
  dReal get_v()  const { return this->v;  }
  dReal get_av() const { return this->av; }

  void order_xy(dReal x, dReal y, bool rel=false);
  void order_a(dReal a, bool rel=false);
  void order_xya(dReal x, dReal y, dReal a, bool rel=false) { order_xy(x,y,rel); order_a(a,rel); }
  void order_back(dReal d);
  void order_stop() { order = ORDER_NONE; }

  bool is_waiting() { return order == ORDER_NONE; }
  //@}

  /// Set linear acceleration
  void set_dv_max(dReal dv);
  /// Set angular acceleration
  void set_dav_max(dReal dav);

  void set_v_max(dReal v)  { this->v_max  = v; }
  void set_av_max(dReal v) { this->av_max = v; }

  void set_threshold_xy(dReal t) { this->threshold_xy = t; }
  void set_threshold_a(dReal t)  { this->threshold_a  = t; }

private:

  /// Plane joint and angle motor
  dJointID j2D;
  /** @brief Linear motor
   *
   * @note Plane 2D joint has 2 plane motors but it does not use robot
   * direction.
   */
  dJointID jLMotor;

  /** @name Position and velocity values
   *
   * These values are updated after calling \e update().
   */
  //@{
  dReal x, y; ///< Position
  dReal a;    ///< Angle position (radians)
  dReal v;    ///< Velocity
  dReal av;   ///< Angular velocity (radians)
  //@}

  dReal v_max;
  dReal av_max;

  /** @name Order targets
   */
  //@{
  dReal target_x, target_y;
  dReal target_a;
  dReal target_back_x, target_back_y;
  //@}

  /** @brief Order types
   *
   * Orders are given as a bitset.
   *
   * Go back order is priority but does not cancel other orders.
   */
  enum
  {
    ORDER_NONE    =  0,
    ORDER_GO_XY   =  1,  // Position order
    ORDER_GO_A    =  2,  // Angle order
    ORDER_GO_BACK =  4,  // Go back
  };


  int order;

  void set_v(dReal v)  { dJointSetLMotorParam(jLMotor, dParamVel, v); }
  void set_av(dReal v) { dJointSetPlane2DAngleParam(j2D, dParamVel, v); }

  /// Asserv position threshold
  dReal threshold_xy;
  /// Asserv angle threshold
  dReal threshold_a;
};

#endif

