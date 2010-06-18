#include <cstdio>
#include "modules/eurobot2009.h"
#include "modules/eurobot2010.h"
#include "display.h"
#include "physics.h"


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

    // Simulation displayed: control speed
    if( Display::display != NULL && Display::display->isInitialized() ) {
      LOG("**** simulation starts");
      Display::display->run();
    } else {
      LOG("no display: simulation run at full speed");
      LOG("**** simulation starts");
      for(;;) {
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


