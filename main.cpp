#include <cstdio>
#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include "modules/eurobot2009.h"
#include "modules/eurobot2010.h"
#include "display.h"
#include "physics.h"
#include "config.h"


#ifdef WIN32
#include <windows.h>
#include <shellapi.h>

/// Wrapper for the standard main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
  return main(__argc, __argv);
}

#endif


int main(int argc, char **argv)
{
  LuaManager *lm = NULL;

  int ret = 0;
  try
  {
    lm = new LuaManager();

    eurobot2009::LuaEurobotModule module2009;
    module2009.import(lm->get_L(), "eurobot2009");
    eurobot2010::LuaEurobotModule module2010;
    module2010.import(lm->get_L(), "eurobot2010");

    if( argc > 1 )
    {
      LOG("load Lua script: %s", argv[1]);
      lm->do_file(argv[1]);
    }
    else
    {
      LOG("no input script, use default: init.lua");
      lm->do_file("init.lua");
    }
    LOG("Lua script loaded, prepare simulation");

    if( !Physics::physics )
      throw(Error("physics not created"));

    if( !Physics::physics->isInitialized() )
    {
      LOG("physics not initialized: init it");
      Physics::physics->init();
    }

    // Simulation displayed: control speed
    if( Display::display != NULL && Display::display->isInitialized() )
    {
      unsigned int disp_dt = (unsigned int)(1000.0/cfg.fps);
      unsigned int step_dt = (unsigned int)(1000.0*cfg.step_dt);
      unsigned long time;
      unsigned long time_disp, time_step;
      signed long time_wait;

      LOG("**** simulation starts");

      time_disp = time_step = SDL_GetTicks();
      while(1)
      {
        time = SDL_GetTicks();
        if( time >= time_step )
        {
          Physics::physics->step();
          time_step += (unsigned long)(step_dt * cfg.time_scale);
        }
        if( time >= time_disp )
        {
          Display::display->processEvents();
          Display::display->update();
          time_disp = time + disp_dt;
        }

        time_wait = MIN(time_step,time_disp);
        time_wait -= time;
        if( time_wait > 0 )
          SDL_Delay(time_wait);
      }
    }
    else
    {
      LOG("no display: simulation run at full speed");
      LOG("**** simulation starts");

      for(;;)
      {
        Physics::physics->step();
      }
    }
  }
  catch(int i)
  {
    LOG("EXIT (%d)", i);
    ret = i;
  }
  catch(const Error &e)
  {
    fprintf(stderr,"%s\n", e.what());
    ret = 1;
  }

  Physics::physics = NULL;
  Display::display = NULL;
  delete lm;

  return ret;
}


