#ifndef GLOBAL_H
#define GLOBAL_H

#include "bullet.h"

#include "maths.h"
#include "colors.h"
#include "log.h"
#include "lua_utils.h"
#include "match.h"
#include "object.h"
#include "robot.h"
#include "physics.h"
#include "display.h"
#include "config.h"

//@file



/** @name Global variable instances
 *
 * Associated classes are accessed via these singletons
 * 
 * @note These variables are defined in \e main.cpp
 */
//@{
extern Config  *cfg;
extern Physics *physics;
extern Display *display;
extern Match   *match;
extern LuaManager *lm;
extern Log *LOG; // lowercase log is already defined :(
//@}

#endif
