#include <stdlib.h>
#include <time.h>

#include "global.h"
#include "colors.h"
#include "robot.h"


Rules::Rules(const Color4 colors[TEAM_NB], int duration)
{
  // Initialize random
  srand(time(NULL));

  this->duration = duration;
  int i;

  for( i=0; i<TEAM_NB; i++ )
    COLOR_COPY(this->colors[i], colors[i]);
}

Rules::~Rules()
{
}

void Rules::check_robots()
{
  // Check robots
  std::vector<Robot*> &robots = Robot::get_robots();
  if( robots.size() > TEAM_NB )
    throw(Error("Too many robots (max %d, got %d)", TEAM_NB, robots.size()));

  // Set teams
  for( unsigned int i=0; i<TEAM_NB && i<robots.size(); i++ )
    robots[i]->set_team(i);
}

const dReal Rules::table_size_x;
const dReal Rules::table_size_y;

