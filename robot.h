#ifndef ROBOT_H
#define ROBOT_H

#include <SDL/SDL.h>
#include <ode/ode.h>
#include <math.h>
#include <vector>

#include "object.h"


/** @brief Basic robot
 *
 * @note Only TEAM_NB robots can be created.
 */
class Robot: public ObjectDynamic
{
public:

  Robot(dGeomID geom, dBodyID body);
  Robot(dGeomID geom, dReal m);

  ~Robot();

  int get_team() { return this->team; }
  void set_team(int i) { this->team = i; }

  virtual void draw();

  /// Draw a small direction cone above the robot
  void draw_direction();

  /** @brief Init robot for the match.
   *
   * Get and cache update and asserv Lua functions, if any.
   * This function should be called before the match starts.
   */
  void init();

  /** @brief Update asserv and strategy data
   *
   * Call the cached update Lua function (if any) or the do_update method.
   *
   * This function is called after ODE step to update internal data using ODE
   * data (e.g. position).
   */
  void update();

  /** @brief Asserv step
   *
   * Call the cached asserv Lua function (if any) or the do_asserv method.
   *
   * Search for a Lua asserv function and execute it.
   */
  void asserv();

  /// Default not implemented do_update function
  virtual void do_update() { throw(Error("do_update() not implemented, use Lua.")); }
  /// Default not implemented do_asserv function
  virtual void do_asserv() { throw(Error("do_sserv() not implemented, use Lua.")); }

  void set_ref_obj(int ref) { this->ref_obj = ref; }

  static std::vector<Robot*> &get_robots() { return robots; }

private:

  /// Robot array
  static std::vector<Robot*> robots;

  /// Common constructor initializations
  void ctor_init();

  /// Team number (-1 if not set)
  int team;

  /// Lua instance reference
  int ref_obj;

  /// Cached update function reference
  int ref_update;

  /// Cached asserv function reference
  int ref_asserv;
};


/** @brief Basic robot
 *
 * A robot with a simple asserv.
 */
class RBasic: public Robot
{
public:

  RBasic(dGeomID geom, dBodyID body);
  RBasic(dGeomID geom, dReal m);

  /** @brief Convenient constructor
   *
   * Create a default box robot with given size and mass.
   */
  RBasic(dReal lx, dReal ly, dReal lz, dReal m);

  ~RBasic();

  /** @brief Update position and velocity values
   */
  virtual void do_update();

  /// Asserv step
  virtual void do_asserv();

  /** @name Strategy methods
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

  /// Common constructor initializations
  void ctor_init();

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

