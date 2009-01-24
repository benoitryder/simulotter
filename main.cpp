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
  int argc;
  char **argv;

  argv = (char **)CommandLineToArgvW(GetCommandLineW(), &argc);
  return main(argc, argv);
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
    LOG->trace("create display");
    display = new Display();

    LOG->trace("create physics");
    physics = new Physics();

    LOG->trace("load Lua script");
    if( argc > 1 )
      lm->do_file(argv[1]);
    else
    {
      LOG->trace("no input script, use default: init.lua");
      lm->do_file("init.lua");
    }

    if( match == NULL )
      throw(Error("no created match"));

    LOG->trace("init physics");
    physics->init();

    LOG->trace("init display");
    glutInit(&argc, argv);
    display->init();

    LOG->trace("init match");
    match->init();

    /*XXX-debug
     */
    if(0){
      // add box shape (will be reused for all bodies)
      btCollisionShape *shape = new btBoxShape(scale(btVector3(0.1,0.1,0.1)));
      btVector3 localInertia(0,0,0);
      shape->calculateLocalInertia(1.0,localInertia);

      // add dynamic rigid body A1
      Object *o1 = new Object();
      o1->setShape(shape);
      o1->setMass(1.0);
      o1->setPos( scale(btVector3(-0.5,0,1)) );
      o1->addToWorld(physics);

      // add dynamic rigid body B1
      Object *o2 = new Object();
      o2->setShape(shape);
      o2->setMass(1.0);
      o2->setPos( scale(btVector3(0.5,0,1)) );
      o2->addToWorld(physics);

      // create slider constraint between A1 and B1 and add it to world
      btTransform frameInA, frameInB;
      frameInA = btTransform::getIdentity();
      frameInB = btTransform::getIdentity();

      btSliderConstraint *slider = new btSliderConstraint(*o1, *o2, frameInA, frameInB, true);
      slider->setLowerLinLimit( scale(-0.5) );
      slider->setUpperLinLimit( scale(+0.5) );
      slider->setLowerAngLimit(1);
      slider->setUpperAngLimit(0);

      physics->getWorld()->addConstraint(slider, false);

      o2->setLinearVelocity( scale(btVector3(0,1,2)) );
    }



    LOG->trace("init robots");
    std::map<unsigned int,Robot*> &robots = match->getRobots();
    std::map<unsigned int,Robot*>::iterator itr;
    for( itr=robots.begin(); itr!=robots.end(); ++itr )
      (*itr).second->matchInit();

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


