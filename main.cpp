#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <unistd.h>
#include <sys/time.h>

#include "global.h"


#ifdef WIN32
#include <windows.h>
#include <shellapi.h>

/// Wrapper for the standard main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
  return main(__argc, __argv);
}

#endif


/// Return current time in milliseconds
unsigned long millitime(void)
{
  struct timeval t;
  gettimeofday(&t,NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}


Config  *cfg     = NULL;
Physics *physics = NULL;
Display *display = NULL;
Match   *match   = NULL;
LuaManager *lm   = NULL;
Log *LOG         = NULL;


int main(int argc, char **argv)
{
  LOG = new Log();
  cfg = new Config();
  lm = new LuaManager();

  try
  {
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

    if( physics == NULL )
      throw(Error("physics not created"));

    if( match == NULL )
      throw(Error("no created match"));

    if( !physics->isInitialized() )
    {
      LOG->trace("physics not initialized: init it");
      physics->init();
    }

    //TODO does not need to be here anymore(?)
    LOG->trace("init match");
    match->init(cfg->match_fconf);


    LOG->trace("init robots");
    std::map<unsigned int,Robot*> &robots = match->getRobots();
    std::map<unsigned int,Robot*>::iterator itr;
    for( itr=robots.begin(); itr!=robots.end(); ++itr )
      (*itr).second->matchInit();

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

      time_disp = time_step = millitime();
      while(1)
      {
        time = millitime();
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
        {
#ifdef WIN32
          Sleep(time_wait);
#else
          usleep(time_wait*1000);
#endif
        }
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
    return i;
  }
  catch(Error e)
  {
    fprintf(stderr,"%s\n", e.what());
    delete physics;
    delete display;
    delete match;
    delete lm;
    delete cfg;
    delete LOG;
    return 1;
  }

  delete physics;
  delete display;
  delete match;
  delete lm;
  delete cfg;
  delete LOG;

  return 0;
}


