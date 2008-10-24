#ifndef CONFIG_H
#define CONFIG_H

#include <ode/ode.h>
#include "lua_utils.h"
#include "colors.h"


/// Gravity at the Earth's surface
#define CONST_EARTH_GRAVITY   9.80665f


/** @brief Configuration class
 *
 * Configurations values are initialized in the constructor but can be changed
 * in the initialisation LUA script.
 *
 * All values are public but they should (or must) not be modified after
 * initialization.
 */
class Config
{
public:

  /** @name Physical parameters
   */
  //@{

  float gravity_z;

  /// Time interval between each simulation step
  float step_dt;
  /// Number of contact points
  int contacts_nb;

  /// Default droping height gap for dynamic objects
  float drop_epsilon;

  //@}


  /** @name SDL drawing
   */
  //@{

  /// Gap size between to contiguous surfaces
  float draw_epsilon;
  /// Slices and stacks for GLUT geometry objects
  int draw_div;
  /// Robot direction cone radius
  float draw_direction_r;
  /// Robot direction cone height
  float draw_direction_h;
  /// Perspective field of view (in degrees)
  float perspective_fov;
  /// Near clipping plance distance
  float perspective_near;
  /// Far clipping plance distance
  float perspective_far;

  //@}

  /** @brief Display settings
   */
  //@{

  int screen_x;
  int screen_y;
  bool fullscreen;

  /** @brief Refresh rate (frames per seconde
   *
   * @note It also defines SDL event handling rate.
   */
  float fps;

  Color4 bg_color;

  //@}

  /// Flush logs after each write
  bool log_flush;

public:
  Config();

public:

  /** @name Lua stuff
   *
   * Config access is provided through fields of the global userdata \c cfg.
   * Access is controlled using \e index and \e newindex metamethods.
   */
  //@{

  /// Init Lua config stuff
  void lua_init(lua_State *L);

  /** @brief \e index metamethod
   *
   * Push the associated config value, raise a lua error for invalid names.
   */
  static int lua_index(lua_State *L);

  /** @brief \e newindex metamethod
   *
   * Push the associated config value, raise a lua error for invalid names.
   */
  static int lua_newindex(lua_State *L);

private:

  /// Metatable name in registry
  static const char *registry_name;

  //@}
};

#endif
