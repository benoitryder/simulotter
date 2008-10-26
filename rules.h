#ifndef RULES_H
#define RULES_H

#include "object.h"
#include "robot.h"

///@file


/// Default duration time
#define DURATION_DEFAULT 90

/// Team number
#define TEAM_NB  2


class Rules
{
public:

  /** @brief Constructor
   *
   * @param  colors    team colors
   * @param  duration  match duration
   */
  Rules(const Color4 colors[TEAM_NB], int duration=DURATION_DEFAULT);
  virtual ~Rules();

  /// Check robots and set teams
  void check_robots();

  /// Get team color
  const GLfloat *get_color(int team) const { return this->colors[team]; }

  /// Get team score
  virtual int get_score(int team) = 0;

  /** @brief Initialize the match
   *
   * Create and initialize game objects.
   *
   * @param  fconf  field fconfuration, -1 for random
   *
   * @note Random seed is intialized in constructor.
   *
   * @todo Check robot number, start position, etc.
   */
  virtual void init(int fconf) = 0;


  static const dReal table_size_x = 3.0;
  static const dReal table_size_y = 2.1;

protected:

  /// Team colors
  Color4 colors[TEAM_NB];

  /// Match duration, in seconds
  unsigned int duration;

};


#endif
