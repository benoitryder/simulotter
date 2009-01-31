#ifndef MATCH_H
#define MATCH_H

#include <map>

/** @brief Invalid team value
 *
 * This value is used as <em>no team</em> value.
 */
#define TEAM_INVALID  ((unsigned int)-1)

#include "global.h"

class Robot;

///@file


/** @brief Game match.
 *
 * Only one match can be created and is automatically assigned to the global
 * \e match variable. It is required to allow match creation in Lua.
 */
class Match
{
  friend class LuaMatch;
public:

  /** @brief Constructor
   *
   * @param  colors    team colors
   * @param  team_nb   number of teams
   * @param  duration  match duration (in seconds)
   *
   * @note Team colors are used to set number of teams.
   */
  Match(const Color4 colors[], unsigned int team_nb=2, int duration=90);
  virtual ~Match();

  unsigned int getTeamNb() const { return team_nb; }

  /** @brief Register a robot and return its team
   *
   * If \e team is not a valid team an available one is used.
   * If the given team is unavailable or if all teams are already
   * used an exception will be thrown.
   *
   * @param  r     robot to register
   * @param  team  team to use
   *
   * @return Robot's team
   */
  unsigned int registerRobot(Robot *r, unsigned int team=TEAM_INVALID);

  std::map<unsigned int, Robot*> &getRobots() { return robots; }

  /// Get color of a given team
  const Color4 getColor(unsigned int team) const { return colors[team]; }

  /** @brief Initialize the match
   *
   * Create and initialize game objects.
   *
   * Call the \e init Lua function (if any) or the do_init() method.
   *
   * @param  fconf  field configuration, -1 for default
   */
  void init(int fconf=-1);

protected:
  /// Default not implemented do_init() method
  virtual void do_init(int fconf) { throw(Error("do_init() not implemented")); }

private:

  /// Number of teams
  unsigned int team_nb;

  /** @brief Registered robots
   * Robots are indexed by their team.
   */
  std::map<unsigned int, Robot*> robots;

  /// Team colors
  Color4 *colors;

  /// Match duration, in seconds
  unsigned int duration;

protected:
  /// Lua instance reference
  int ref_obj;
};


#endif

