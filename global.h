#ifndef GLOBAL_H
#define GLOBAL_H

#include "config.h"
#include "physics.h"
#include "display.h"
#include "rules.h"
#include "lua_utils.h"
#include "log.h"


/** @name Global variable instance
 *
 * Associated classes are accessed via these singletons
 * 
 * @note These variables are defined in \e main.cpp
 */
//@{

extern Config  *cfg;
extern Physics *physics;
extern Display *display;
extern Rules   *rules;
extern LuaManager *lm;
extern Log *LOG; // lowercase log is already defined :(

//@}

#endif
