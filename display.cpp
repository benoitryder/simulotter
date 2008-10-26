#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <GL/freeglut.h>
#include <math.h>
#include <vector>
#include "global.h"
#include "colors.h"
#include "robot.h"
#include "maths.h"
#include "icon.h"



Display::Display()
{
  this->screen = NULL;
  this->sdl_bpp   = 0;
  this->sdl_flags = 0;

  window_init();
  scene_init();

  camera.mode = CAM_FIXED;
  camera.eye[0] = 4;
  camera.eye[1] = M_PI/6;
  camera.eye[2] = -M_PI/3;
  camera.target[0] = 0;
  camera.target[1] = 0;
  camera.target[2] = 0;
}

Display::~Display()
{
  window_destroy();
  scene_destroy();
}


void Display::resize(int width, int height)
{
  LOG->trace("Display resize: %d x %d", width, height);
  if( (screen = SDL_SetVideoMode(width, height, sdl_bpp, sdl_flags)) == NULL )
  {
    window_destroy();
    throw(Error("SDL: cannot change video mode"));
  }

  this->screen_x = width;
  this->screen_y = height;

  scene_init();
}


void Display::update()
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_RESCALE_NORMAL);
  glEnable(GL_COLOR_MATERIAL);

  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity();
  gluPerspective(cfg->perspective_fov, ((float)screen_x)/screen_y,
      cfg->perspective_near, cfg->perspective_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera position
  float *spherical, *eye, *target;
  float xyz[3];
  if( camera.mode == CAM_FIXED )
  {
    spherical = camera.eye;
    eye = xyz;
    target = camera.target;
    for( i=0; i<3; i++ )
      xyz[i] = target[i];
  }
  else
  {
    spherical = camera.target;
    eye = camera.eye;
    target = xyz;
    for( i=0; i<3; i++ )
      xyz[i] = eye[i];
  }

  spheric2cart_add(xyz, spherical[0], spherical[1], spherical[2]);

  gluLookAt(
      eye[0], eye[1], eye[2],
      target[0], target[1], target[2],
      0.0, 0.0, (((int)ceil(spherical[1]/M_PI))%2 == 0) ? -1.0 : 1.0);

  // Draw objects
  std::vector<Object*> &objs = physics->get_objs();
  std::vector<Object*>::iterator it;
  for( it = objs.begin(); it != objs.end(); it++ )
  {
    if( (*it)->is_visible() )
      (*it)->draw();
  }

  //XXX-hackboxes Display cylinder-hack boxes
  std::vector<dGeomID>::iterator it2;
  for( it2=physics->hack_boxes.begin(); it2!=physics->hack_boxes.end(); it2++ )
  {
    dGeomID geom = *it2;
    glPushMatrix();
    glColor3f(1, 1, 0);

    const dReal *pos = dGeomGetPosition(geom);
    const dReal *rot = dGeomGetRotation(geom);

    GLfloat m[16] = {
      rot[0], rot[4], rot[8],  0.0f,
      rot[1], rot[5], rot[9],  0.0f,
      rot[2], rot[6], rot[10], 0.0f,
      pos[0], pos[1], pos[2],  1.0f
    };
    glMultMatrixf(m);

    // Offset

    // XXX Set rotation offset should only change geom rotation, but body
    // rotation is modified too. Whe use offset to rotate a cylinder around its
    // axis, thus it's not a problem.
    pos = dGeomGetOffsetPosition(geom);
    glTranslatef(pos[0], pos[1], pos[2]);
    /*
    rot = dGeomGetOffsetRotation(geom);
    GLfloat m2[16] = {
      rot[0], rot[4], rot[8],  0.0f,
      rot[1], rot[5], rot[9],  0.0f,
      rot[2], rot[6], rot[10], 0.0f,
      pos[0], pos[1], pos[2],  1.0f
    };
    glMultMatrixf(m2);
    */

    dVector3 size;
    dGeomBoxGetLengths(geom, size);
    glScalef(size[0], size[1], size[2]);
    glutSolidCube(1.0f);

    glPopMatrix();
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, screen_x, 0.0, screen_y);

  SDL_GL_SwapBuffers();
  glFlush();
}


void Display::window_init()
{
  const SDL_VideoInfo* info = NULL;

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    window_destroy();
    throw(Error("SDL: initialization failed"));
  }

  // Get the best available bpp value
  if( (info = SDL_GetVideoInfo()) == NULL )
  {
    window_destroy();
    throw(Error("SDL: cannot get hardware info"));
  }
  this->sdl_bpp = info->vfmt->BitsPerPixel;

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if(cfg->fullscreen)
    this->sdl_flags = SDL_OPENGL | SDL_FULLSCREEN;
  else
    this->sdl_flags = SDL_OPENGL | SDL_RESIZABLE;

  SDL_WM_SetCaption("SimulOtter", NULL);
  
  LOG->trace("ICON");
  SDL_WM_SetIcon( SDL_CreateRGBSurfaceFrom(
        icon.data, icon.width, icon.height, 8*icon.bpp, icon.bpp*icon.width,
        0xff, 0xff<<8, 0xff<<16, 0xff<<24),
      NULL);

  resize(cfg->screen_x, cfg->screen_y);
}

void Display::window_destroy()
{
  SDL_Quit();
}


void Display::scene_init()
{
  // Clear the background color
  glClearColor(cfg->bg_color[0], cfg->bg_color[1],
      cfg->bg_color[2], cfg->bg_color[3]);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_DEPTH_TEST); 
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}

void Display::scene_destroy() {}



void Display::handle_events()
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_QUIT:
        throw(0);
        break;
      case SDL_VIDEORESIZE:
        resize(event.resize.w, event.resize.h);
        break;

      case SDL_KEYDOWN:
        switch( event.key.keysym.sym )
        {
          case SDLK_SPACE:
            physics->toggle_pause();
            break;
          case SDLK_c:
            LOG->trace("change camera mode");
            if( camera.mode == CAM_FIXED )
              set_camera_mode(CAM_FREE);
            else
              set_camera_mode(CAM_FIXED);
            break;
          default:
            break;
        }
        break;

      case SDL_MOUSEMOTION:
        if( (event.motion.state & SDL_BUTTON(1)) == 0 )
          break;
        //XXX-config
        if( camera.mode == CAM_FIXED )
        {
          camera.eye[1] += event.motion.yrel * 0.01;
          camera.eye[2] -= event.motion.xrel * 0.01;
        }
        else
        {
          camera.target[1] += event.motion.yrel * 0.01;
          camera.target[2] -= event.motion.xrel * 0.01;
        }
        break;
    }
  }

  // Pressed keys

  Uint8 *keys;
  keys = SDL_GetKeyState(NULL);

  if( keys[SDLK_ESCAPE] )
    throw(0);
  //XXX-config
  if( camera.mode == CAM_FIXED )
  {
    if( keys[SDLK_LEFT] || keys[SDLK_a] )
      camera.eye[2] -= 0.1;
    if( keys[SDLK_RIGHT] || keys[SDLK_d] )
      camera.eye[2] += 0.1;
    if( keys[SDLK_e] )
      camera.eye[1] -= 0.1;
    if( keys[SDLK_q] )
      camera.eye[1] += 0.1;
    if( keys[SDLK_UP] || keys[SDLK_w] )
      camera.eye[0] -= 0.1;
    if( keys[SDLK_DOWN] || keys[SDLK_s] )
      camera.eye[0] += 0.1;
  }
  else if( camera.mode == CAM_FREE )
  {
    if( keys[SDLK_LEFT] || keys[SDLK_a] )
      spheric2cart_add(camera.eye, 0.1, M_PI_2, camera.target[2]+M_PI_2);
    if( keys[SDLK_RIGHT] || keys[SDLK_d] )
      spheric2cart_add(camera.eye, -0.1, M_PI_2, camera.target[2]+M_PI_2);
    if( keys[SDLK_e] )
      spheric2cart_add(camera.eye, 0.1, camera.target[1]-M_PI_2, camera.target[2]);
    if( keys[SDLK_q] )
      spheric2cart_add(camera.eye, -0.1, camera.target[1]-M_PI_2, camera.target[2]);
    if( keys[SDLK_UP] || keys[SDLK_w] )
      spheric2cart_add(camera.eye, 0.1, camera.target[1], camera.target[2]);
    if( keys[SDLK_DOWN] || keys[SDLK_s] )
      spheric2cart_add(camera.eye, -0.1, camera.target[1], camera.target[2]);
  }
}


void Display::set_camera_mode(CameraMode mode)
{
  if( camera.mode == mode )
    return;

  if( camera.mode == CAM_FIXED && mode == CAM_FREE )
  {
    float eye[3];
    eye[0] = camera.target[0];
    eye[1] = camera.target[1];
    eye[2] = camera.target[2];
    spheric2cart_add(eye, camera.eye[0], camera.eye[1], camera.eye[2]);
    camera.eye[0] = eye[0];
    camera.eye[1] = eye[1];
    camera.eye[2] = eye[2];
    cart2spheric(camera.target, camera.target[0]-camera.eye[0], camera.target[1]-camera.eye[1], camera.target[2]-camera.eye[2]);
  }
  else if( camera.mode == CAM_FREE && mode == CAM_FIXED )
  {
    //XXX fix on a given object/position
    cart2spheric(camera.eye, camera.eye[0], camera.eye[1], camera.eye[2]);
    camera.target[0] = 0;
    camera.target[1] = 0;
    camera.target[2] = 0;
  }

  camera.mode = mode;
}


