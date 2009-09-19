#ifndef GLOBAL_H
#define GLOBAL_H

#include "bullet.h"
#include "maths.h"
#include "colors.h"
#include "log.h"
#include "smart.h"
#include "lua_utils.h"
#include "physics.h"
#include "object.h"
#include "sensors.h"
#include "robot.h"
#include "config.h"
#include "display.h"

//@file



/** @name Global variable instances
 *
 * Associated classes are accessed via these singletons
 * 
 * @note These variables are defined in \e main.cpp
 */
//@{
extern Config *cfg;
extern LuaManager *lm;
extern Log *LOG; // lowercase log is already defined :(
extern SmartPtr<Physics> physics;
extern SmartPtr<Display> display;
//@}

#endif
