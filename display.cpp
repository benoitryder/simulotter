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

  camera.mode = CAM_FIXED;
  camera.eye[0] = 4;
  camera.eye[1] = M_PI/6;
  camera.eye[2] = -M_PI/3;
  camera.target[0] = 0;
  camera.target[1] = 0;
  camera.target[2] = 0;


  // Default handlers

  SDL_Event event;
  
  // Window events
  event.type = SDL_QUIT;
  set_handler(event, handler_quit);
  event.type = SDL_VIDEORESIZE;
  set_handler(event, handler_resize);

  // Mouse motion
  event.type = SDL_MOUSEMOTION;
  event.motion.state = SDL_BUTTON(1);
  set_handler(event, handler_cam_mouse);

  // Keyboard

  event.key.keysym.sym = SDLK_ESCAPE;
  event.type = SDL_KEYDOWN;
  set_handler(event, handler_quit);

  event.key.keysym.sym = SDLK_SPACE;
  event.type = SDL_KEYUP;
  set_handler(event, handler_pause);

  event.key.keysym.sym = SDLK_c;
  event.type = SDL_KEYUP;
  set_handler(event, handler_cam_mode);

  // Camera moves
  // TODO correct AZERTY/QWERTY handling
  event.type = SDL_KEYDOWN;
#ifdef WIN32
  event.key.keysym.sym = SDLK_w;
#else
  event.key.keysym.sym = SDLK_z;
#endif
  set_handler(event, handler_cam_ahead);
  event.key.keysym.sym = SDLK_s;
  set_handler(event, handler_cam_back);
#ifdef WIN32
  event.key.keysym.sym = SDLK_a;
#else
  event.key.keysym.sym = SDLK_q;
#endif
  set_handler(event, handler_cam_left);
  event.key.keysym.sym = SDLK_d;
  set_handler(event, handler_cam_right);
#ifdef WIN32
  event.key.keysym.sym = SDLK_q;
#else
  event.key.keysym.sym = SDLK_a;
#endif
  set_handler(event, handler_cam_up);
  event.key.keysym.sym = SDLK_e;
  set_handler(event, handler_cam_down);
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_RESCALE_NORMAL);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity();
  gluPerspective(cfg->perspective_fov, ((float)screen_x)/screen_y,
      cfg->perspective_near, cfg->perspective_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  const GLfloat light_pos[4] = {0,.3,.8,0};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  // Camera position
  float eye_pos[3], target_pos[3];
  get_camera_pos(eye_pos, target_pos);

  float up = (
      ( (camera.mode & CAM_EYE_REL) && ((int)ceil(camera.eye[1]/M_PI))%2==0 ) ||
      ( (camera.mode & CAM_TARGET_REL) && ((int)ceil(camera.target[1]/M_PI))%2==0 )
      ) ? -1.0 : 1.0;

  gluLookAt(
      eye_pos[0], eye_pos[1], eye_pos[2],
      target_pos[0], target_pos[1], target_pos[2],
      0.0, 0.0, up);

  // Draw objects
  std::vector<Object*> &objs = physics->get_objs();
  std::vector<Object*>::iterator it;
  for( it = objs.begin(); it != objs.end(); ++it )
  {
    if( (*it)->is_visible() )
      (*it)->draw();
  }

  //XXX-hackboxes Display cylinder-hack boxes
  std::vector<dGeomID>::iterator it2;
  for( it2=physics->hack_boxes.begin(); it2!=physics->hack_boxes.end(); ++it2 )
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
    throw(Error("SDL: initialization failed: %s", SDL_GetError()));
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

  SDL_WM_SetIcon( SDL_CreateRGBSurfaceFrom(
        icon.data, icon.width, icon.height, 8*icon.bpp, icon.bpp*icon.width,
        0xff, 0xff<<8, 0xff<<16, 0xff<<24),
      NULL);

  //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableKeyRepeat(10, 5);

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
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}

void Display::scene_destroy() {}



void Display::handle_events()
{
  SDL_Event event;
  std::map<SDL_Event, event_handler, EventCmp>::iterator ithandler;

  while(SDL_PollEvent(&event))
  {
    ithandler = handlers.find(event);
    if( ithandler != handlers.end() )
      (*ithandler).second(this, event);
  }
}



void Display::get_camera_pos(float eye_pos[3], float target_pos[3])
{
  // ref positions depends on the other point.
  // Thus, fixed and mobile positions are retrieved first.

  if( camera.mode & CAM_EYE_FIXED )
  {
    for( int i=0; i<3; i++ )
      eye_pos[i] = camera.eye[i];
  }
  else if( camera.mode & CAM_EYE_OBJECT )
  {
    const dReal *pos = camera.eye_obj->get_pos();
    const dReal *rot = camera.eye_obj->get_rot();

    // Convert offset in global coordinates
    // (Apply object rotation using quaternions.)
    dQuaternion q1, q2;

    q1[0] = 0;
    for( int i=0; i<3; i++ )
      q1[i+1] = camera.eye[i];
    dQMultiply0(q2, rot, q1);
    dQMultiply2(q1, q2, rot);

    for( int i=0; i<3; i++ )
      eye_pos[i] = pos[i] + q1[i+1];
  }

  if( camera.mode & CAM_TARGET_FIXED )
  {
    for( int i=0; i<3; i++ )
      target_pos[i] = camera.target[i];
  }
  else if( camera.mode & CAM_TARGET_OBJECT )
  {
    const dReal *pos = camera.target_obj->get_pos();
    const dReal *rot = camera.target_obj->get_rot();

    // Convert offset in global coordinates
    // (Apply object rotation using quaternions.)
    dQuaternion q1, q2;

    q1[0] = 0;
    for( int i=0; i<3; i++ )
      q1[i+1] = camera.target[i];
    dQMultiply0(q2, rot, q1);
    dQMultiply2(q1, q2, rot);

    for( int i=0; i<3; i++ )
      target_pos[i] = pos[i] + q1[i+1];
  }

  if( camera.mode & CAM_EYE_REL )
  {
    spheric2cart(eye_pos, camera.eye[0], camera.eye[1], camera.eye[2]);

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera.mode & CAM_TARGET_OBJECT )
    {
      const dReal *rot = camera.target_obj->get_rot();
      dQuaternion q1, q2;
      q1[0] = 0;
      for( int i=0; i<3; i++ )
        q1[i+1] = eye_pos[i];
      dQMultiply0(q2, rot, q1);
      dQMultiply2(q1, q2, rot);
      for( int i=0; i<3; i++ )
        eye_pos[i] = q1[i+1];
    }

    for( int i=0; i<3; i++ )
      eye_pos[i] += target_pos[i];
  }
  else if( camera.mode & CAM_TARGET_REL )
  {
    spheric2cart(target_pos, camera.target[0], camera.target[1], camera.target[2]);

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera.mode & CAM_EYE_OBJECT )
    {
      const dReal *rot = camera.eye_obj->get_rot();
      dQuaternion q1, q2;
      q1[0] = 0;
      for( int i=0; i<3; i++ )
        q1[i+1] = target_pos[i];
      dQMultiply0(q2, rot, q1);
      dQMultiply2(q1, q2, rot);
      for( int i=0; i<3; i++ )
        target_pos[i] = q1[i+1];
    }

    for( int i=0; i<3; i++ )
      target_pos[i] += eye_pos[i];
  }
}


void Display::set_camera_mode(int mode)
{
  if( camera.mode == mode )
    return;

  if( (mode & CAM_EYE_OBJECT) && (mode & CAM_TARGET_OBJECT) )
    throw(Error("invalid camera mode"));

  // Set new coordinates
  // Try to keep the same (global) positions

  float eye_pos[3];
  float target_pos[3];

  get_camera_pos(eye_pos, target_pos);

  switch( mode & CAM_EYE_MASK )
  {
    case CAM_EYE_FIXED:
      for( int i=0; i<3; i++ )
        camera.eye[i] = eye_pos[i];
      break;
    case CAM_EYE_REL:
      cart2spheric(camera.eye, eye_pos[0]-target_pos[0], eye_pos[1]-target_pos[1], eye_pos[2]-target_pos[2]);
      break;
    case CAM_EYE_OBJECT:
      if( camera.eye_obj == NULL )
        throw(Error("object must be set before setting an object camera mode"));
      for( int i=0; i<3; i++ )
        camera.eye[i] = 0;
      break;
  }

  switch( mode & CAM_TARGET_MASK )
  {
    case CAM_TARGET_FIXED:
      for( int i=0; i<3; i++ )
        camera.target[i] = target_pos[i];
      break;
    case CAM_TARGET_REL:
      cart2spheric(camera.target, target_pos[0]-eye_pos[0], target_pos[1]-eye_pos[1], target_pos[2]-eye_pos[2]);
      break;
    case CAM_TARGET_OBJECT:
      if( camera.target_obj == NULL )
        throw(Error("object must be set before setting an object camera mode."));
      for( int i=0; i<3; i++ )
        camera.target[i] = 0;
      break;
  }

  camera.mode = mode;
  LOG->trace("camera mode: %x < %x", mode&CAM_EYE_MASK, (mode&CAM_TARGET_MASK)>>8);
}


bool Display::EventCmp::operator()(const SDL_Event &a, const SDL_Event &b)
{
  if( a.type == b.type )
  {
    switch( a.type )
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        return a.key.keysym.sym < b.key.keysym.sym;
      case SDL_MOUSEMOTION:
        return a.motion.state < b.motion.state;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        return a.button.button < b.button.button;
      case SDL_USEREVENT:
        return a.user.code < b.user.code;
      default: // events are equal
        return false;
    }
  }

  return a.type < b.type;
}


void Display::handler_resize(Display *d, const SDL_Event &event)
{
  d->resize(event.resize.w, event.resize.h);
}

void Display::handler_pause(Display *d, const SDL_Event &event)
{
  physics->toggle_pause();
}

void Display::handler_cam_mode(Display *d, const SDL_Event &event)
{
  switch( d->get_camera_mode() )
  {
    case CAM_FIXED:
      d->set_camera_mode(CAM_FREE);
      break;
    case CAM_FREE:
      {
        //TODO allow other objects
        std::map<unsigned int,Robot*> &robots = match->get_robots();
        if( !robots.empty() )
        {
          d->set_camera_target_obj((*robots.begin()).second);
          d->set_camera_mode(CAM_LOOK);
          break;
        }
      }
    case CAM_LOOK:
      {
        //TODO allow other objects
        std::map<unsigned int,Robot*> &robots = match->get_robots();
        if( !robots.empty() )
        {
          d->set_camera_eye_obj((*robots.begin()).second);
          d->set_camera_mode(CAM_ONBOARD);
          d->set_camera_target(1.0, 3*M_PI/4, 0.0);//XXX
          d->set_camera_eye(0.0, 0.0, 0.3);//XXX
          break;
        }
      }
    case CAM_ONBOARD:
    default:
      d->set_camera_mode(CAM_FIXED);
      break;
  }
}

void Display::handler_cam_mouse(Display *d, const SDL_Event &event)
{
  float dx = event.motion.xrel * cfg->camera_mouse_coef;
  float dy = event.motion.yrel * cfg->camera_mouse_coef;
  if( d->camera.mode & CAM_EYE_REL )
    d->move_camera_eye( 0, dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
  else if( d->camera.mode & CAM_TARGET_REL )
    d->move_camera_target( 0, dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
}


void Display::handler_cam_ahead(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, cfg->camera_step_linear, d->camera.target[1], d->camera.target[2]);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[0] -= cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[0] -= cfg->camera_step_linear;
}

void Display::handler_cam_back(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, -cfg->camera_step_linear, d->camera.target[1], d->camera.target[2]);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[0] += cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[0] += cfg->camera_step_linear;
}

void Display::handler_cam_left(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, cfg->camera_step_linear, M_PI_2, d->camera.target[2]+M_PI_2);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[2] -= cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[2] -= cfg->camera_step_linear;
}

void Display::handler_cam_right(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, -cfg->camera_step_linear, M_PI_2, d->camera.target[2]+M_PI_2);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[2] += cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[2] += cfg->camera_step_linear;
}

void Display::handler_cam_up(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, -cfg->camera_step_linear, d->camera.target[1]-M_PI_2, d->camera.target[2]);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[1] -= cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[1] -= cfg->camera_step_linear;
}

void Display::handler_cam_down(Display *d, const SDL_Event &event)
{
  if( d->camera.mode == CAM_FREE )
    spheric2cart_add(d->camera.eye, cfg->camera_step_linear, d->camera.target[1]-M_PI_2, d->camera.target[2]);
  else if( d->camera.mode & CAM_EYE_REL )
    d->camera.eye[1] += cfg->camera_step_linear;
  else if( d->camera.mode & CAM_TARGET_REL )
    d->camera.target[1] += cfg->camera_step_linear;
}

