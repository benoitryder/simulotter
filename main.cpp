#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/freeglut.h>

#include "global.h"
#include "modules/eurobot2009.h"
#include "modules/eurobot2010.h"


#ifdef WIN32
#include <windows.h>
#include <shellapi.h>

/// Wrapper for the standard main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
  return main(__argc, __argv);
}

#endif


Config  *cfg     = NULL;
LuaManager *lm   = NULL;
Log *LOG         = NULL;
SmartPtr<Physics> physics;
SmartPtr<Display> display;


int main(int argc, char **argv)
{
  LOG = new Log();

  int ret = 0;
  try
  {
    cfg = new Config();
    lm = new LuaManager();

    eurobot2009::LuaEurobotModule module2009;
    module2009.import(lm->get_L(), "eurobot2009");
    eurobot2010::LuaEurobotModule module2010;
    module2010.import(lm->get_L(), "eurobot2010");

    if( argc > 1 )
    {
      LOG->trace("load Lua script: %s", argv[1]);
      lm->do_file(argv[1]);
    }
    else
    {
      LOG->trace("no input script, use default: init.lua");
      lm->do_file("init.lua");
    }
    LOG->trace("Lua script loaded, prepare simulation");

    if( !physics )
      throw(Error("physics not created"));

    if( !physics->isInitialized() )
    {
      LOG->trace("physics not initialized: init it");
      physics->init();
    }

    // Simulation displayed: control speed
    if( display != NULL )
    {
      if( !display->isInitialized() )
      {
        LOG->trace("display created but not initialized: init it");
        display->init();
      }

      unsigned int disp_dt = (unsigned int)(1000.0/cfg->fps);
      unsigned int step_dt = (unsigned int)(1000.0*cfg->step_dt);
      unsigned long time;
      unsigned long time_disp, time_step;
      signed long time_wait;

      LOG->trace("**** simulation starts");

      time_disp = time_step = SDL_GetTicks();
      while(1)
      {
        time = SDL_GetTicks();
        if( time >= time_step )
        {
          physics->step();
          time_step += (unsigned long)(step_dt * cfg->time_scale);
        }
        if( time >= time_disp )
        {
          display->handleEvents();
          display->update();
          time_disp += disp_dt;
        }

        time_wait = MIN(time_step,time_disp);
        time_wait -= time;
        if( time_wait > 0 )
          SDL_Delay(time_wait);
      }
    }
    else
    {
      LOG->trace("no display: simulation run at full speed");
      LOG->trace("**** simulation starts");

      for(;;)
      {
        physics->step();
      }
    }
  }
  catch(int i)
  {
    LOG->trace("EXIT");
    ret = i;
  }
  catch(Error e)
  {
    fprintf(stderr,"%s\n", e.what());
    ret = 1;
  }

  display = NULL;
  physics = NULL;
  delete lm;
  delete cfg;
  delete LOG;

  return ret;
}


