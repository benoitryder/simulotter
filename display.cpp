#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_endian.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <png.h>
#include <vector>
#include "global.h"
#include "icon.h"



Display::Display()
{
  int argc = 1;
  glutInit(&argc, NULL);

  this->screen = NULL;
  this->screenshot_filename = NULL;

  camera_mode = CAM_FIXED;
  camera_eye.spheric = scale( btSpheric3(4.0, M_PI/6, -M_PI/3) );
  camera_target.cart = btVector3(0,0,0);
  camera_eye.obj    = NULL;
  camera_target.obj = NULL;

  // Default handlers

  SDL_Event event;
  
  // Window events
  event.type = SDL_QUIT;
  setHandler(event, handlerQuit);
  event.type = SDL_VIDEORESIZE;
  setHandler(event, handlerResize);

  // Mouse motion
  event.type = SDL_MOUSEMOTION;
  event.motion.state = SDL_BUTTON(1);
  setHandler(event, handlerCamMouse);

  // Keyboard

  event.key.keysym.sym = SDLK_ESCAPE;
  event.type = SDL_KEYDOWN;
  setHandler(event, handlerQuit);

  event.key.keysym.sym = SDLK_SPACE;
  event.type = SDL_KEYUP;
  setHandler(event, handlerPause);

  // Camera moves
  // TODO correct AZERTY/QWERTY handling
  event.type = SDL_KEYDOWN;
#ifdef WIN32
  event.key.keysym.sym = SDLK_w;
#else
  event.key.keysym.sym = SDLK_z;
#endif
  setHandler(event, handlerCamAhead);
  event.key.keysym.sym = SDLK_s;
  setHandler(event, handlerCamBack);
#ifdef WIN32
  event.key.keysym.sym = SDLK_a;
#else
  event.key.keysym.sym = SDLK_q;
#endif
  setHandler(event, handlerCamLeft);
  event.key.keysym.sym = SDLK_d;
  setHandler(event, handlerCamRight);
#ifdef WIN32
  event.key.keysym.sym = SDLK_q;
#else
  event.key.keysym.sym = SDLK_a;
#endif
  setHandler(event, handlerCamUp);
  event.key.keysym.sym = SDLK_e;
  setHandler(event, handlerCamDown);
}

Display::~Display()
{
  windowDestroy();
  sceneDestroy();
}

void Display::init()
{
  if( isInitialized() )
  {
    LOG->trace("display already initialized, init() call skipped");
    return;
  }
  windowInit();
}


void Display::resize(int width, int height, int mode)
{
  bool fullscreen;
  if( mode == 0 )
    fullscreen = false;
  else if( mode > 0 )
    fullscreen = true;
  else
    fullscreen = this->fullscreen;

  LOG->trace("Display resize: %d x %d, %s mode",
      width, height, fullscreen ? "fullscreen" : "window");

  Uint32 flags = SDL_OPENGL;
  flags |= fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE;

  if( (screen = SDL_SetVideoMode(width, height, 0, flags)) == NULL )
  {
    windowDestroy();
    throw(Error("SDL: cannot change video mode"));
  }

  this->screen_x = width;
  this->screen_y = height;
  this->fullscreen = fullscreen;

  sceneInit();
}


void Display::update()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity();
  gluPerspective(cfg->perspective_fov, ((float)screen_x)/screen_y,
      cfg->perspective_near, cfg->perspective_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  btVector3 eye_pos, target_pos;
  getCameraPos(eye_pos, target_pos);

  float up = (
      ( (camera_mode & CAM_EYE_REL) &&
        ((int)btCeil(camera_eye.spheric.theta/M_PI))%2==0 ) ||
      ( (camera_mode & CAM_TARGET_REL) &&
        ((int)btCeil(camera_target.spheric.theta/M_PI))%2==0 )
      ) ? -1.0 : 1.0;

  gluLookAt(
      eye_pos[0], eye_pos[1], eye_pos[2],
      target_pos[0], target_pos[1], target_pos[2],
      0.0, 0.0, up);

  // Draw objects
  std::set< SmartPtr<Object> > &objs = physics->getObjs();
  std::set< SmartPtr<Object> >::iterator it_obj;
  for( it_obj = objs.begin(); it_obj != objs.end(); ++it_obj )
    (*it_obj)->draw();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, screen_x, 0.0, screen_y);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // OSD
  glDisable(GL_LIGHTING);
  std::set< SmartPtr<OSDMessage> >::iterator it_osd;
  for( it_osd = osds.begin(); it_osd != osds.end(); ++it_osd )
    drawString( (*it_osd)->getText(),
        (*it_osd)->getX(), (*it_osd)->getY(),
        (*it_osd)->getColor(), GLUT_BITMAP_8_BY_13
        );
  glEnable(GL_LIGHTING);

  SDL_GL_SwapBuffers();
  glFlush();

  // Save screenshot
  if( screenshot_filename != NULL )
  {
    doSavePNGScreenshot(screenshot_filename);
    delete[] screenshot_filename;
    screenshot_filename = NULL;
  }
}


/** @name PNG related declarations.
 */
//@{

#define PNG_ERROR_SIZE  256

typedef struct
{
  char msg[PNG_ERROR_SIZE];
} png_error_data;


static void png_handler_error(png_struct *png_ptr, const char *msg)
{
  png_error_data *error = (png_error_data *)png_get_error_ptr(png_ptr);
  strncpy(error->msg, msg, PNG_ERROR_SIZE);
  error->msg[PNG_ERROR_SIZE-1] = '\0';
  longjmp(png_ptr->jmpbuf, 1);
}

static void png_handler_warning(png_struct *png_ptr, const char *msg)
{
  LOG->trace("PNG: warning: %s", msg);
}

//@}

void Display::savePNGScreenshot(const char *filename)
{
  if( filename == NULL )
    return;
  if( screenshot_filename != NULL )
    delete[] screenshot_filename;
  int n = ::strlen(filename);
  screenshot_filename = new char[n+1];
  strncpy(screenshot_filename, filename, n);
  screenshot_filename[n] = '\0';
}

void Display::doSavePNGScreenshot(const char *filename)
{
  FILE *fp = NULL;
  png_bytep pixels = NULL;
  png_bytepp row_pointers = NULL;

  try
  {
    // Open output file
    FILE *fp = fopen(filename, "wb");
    if( !fp )
      throw(Error("cannot open file '%s' for writing", filename));

    png_error_data error;

    // PNG initializations

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, &error,
        png_handler_error, png_handler_warning
        );
    if( !png_ptr )
      throw(Error("png_create_write struct failed"));

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if( !info_ptr )
    {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      throw(Error("png_create_info_struct failed"));
    }

    if( setjmp(png_ptr->jmpbuf) )
    {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw(Error(error.msg));
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, screen_x, screen_y, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    // Get pixels, create pixel rows
    SDL_LockSurface(this->screen);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    SDL_UnlockSurface(this->screen);
    pixels = new unsigned char[4*screen_x*screen_y];
    glReadPixels(0, 0, screen_x, screen_y, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    row_pointers = new png_bytep[screen_y];
    int i;
    for( i=0; i<screen_y; i++ )
      row_pointers[screen_y-i-1] = (png_bytep)pixels+3*i*screen_x;
    png_set_rows(png_ptr, info_ptr, row_pointers);

    // Write data
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    delete[] row_pointers; row_pointers = NULL;
    delete[] pixels; pixels = NULL;
    fclose(fp); fp = NULL;

    LOG->trace("screenshot saved: %s", filename);
  }
  catch(const Error &e)
  {
    if( row_pointers != NULL )
      delete[] row_pointers;
    if( pixels != NULL )
      delete[] pixels;
      fclose(fp);
    throw(Error("SDL: cannot save screenshot: %s", e.what()));
  }
}


void Display::drawString(const char *s, int x, int y, Color4 color, void *font)
{
  y = screen_y - y;
  glColor4fv(color);
  glRasterPos2f(x,y);
  for( unsigned int i=0; i<strlen(s); i++)
    glutBitmapCharacter(font, s[i]);
}


void Display::windowInit()
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    windowDestroy();
    throw(Error("SDL: initialization failed: %s", SDL_GetError()));
  }

  SDL_WM_SetCaption("SimulOtter", NULL);

  SDL_WM_SetIcon( SDL_CreateRGBSurfaceFrom(
        icon.data, icon.width, icon.height, 8*icon.bpp, icon.bpp*icon.width,
        0xff, 0xff<<8, 0xff<<16, 0xff<<24),
      NULL);

  //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableKeyRepeat(10, 5);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if( cfg->antialias > 0 )
  {
    if( SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) < 0 )
      throw(Error("SDL: cannot enable multisample buffers"));
    if( SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, cfg->antialias) < 0 )
      throw(Error("SDL: cannot set multisample sample count to %d", cfg->antialias));
  }

  resize(cfg->screen_x, cfg->screen_y, cfg->fullscreen);
}

void Display::windowDestroy()
{
  SDL_Quit();
}


void Display::sceneInit()
{
  // Clear the background color
  glClearColor(cfg->bg_color[0], cfg->bg_color[1],
      cfg->bg_color[2], cfg->bg_color[3]);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  const GLfloat light_pos[4] = {0,scale(.3),scale(.8),0};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  glEnable(GL_DEPTH_TEST); 
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);

  //glEnable(GL_RESCALE_NORMAL);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

}

void Display::sceneDestroy() {}



void Display::processEvents()
{
  SDL_Event event;
  EventHandlerContainer::iterator it_h;

  while(SDL_PollEvent(&event))
  {
    it_h = handlers.find(event);
    if( it_h != handlers.end() )
      (*it_h).second(this, &event);
  }
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

void Display::setHandler(const SDL_Event &ev, const EventHandler &h)
{
  handlers.insert( EventHandlerContainer::value_type(ev, h) );
}

Display::EventHandler::EventHandler(lua_State *L, int ref): ptr_cb(NULL), L(L), ref_cb(ref)
{
  if( ref_cb == LUA_NOREF || ref_cb == LUA_REFNIL )
    throw(LuaError(L, "invalid object reference for EventHandler"));
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_cb);
  if( !lua_isfunction(L, -1) )
    throw(LuaError(L, "EventHandler: invalid callback type"));
  lua_pop(L,1);
}

Display::EventHandler::EventHandler(const EventHandler &h): ptr_cb(h.ptr_cb)
{
  if( h.ref_cb != LUA_NOREF )
  {
    // Copy the reference and the state
    L = h.L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, h.ref_cb);
    ref_cb = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else
  {
    L = NULL;
    ref_cb = LUA_NOREF;
  }
}

Display::EventHandler::~EventHandler()
{
  if( L != NULL )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_cb);
}

void Display::EventHandler::operator()(Display *d, const SDL_Event *ev) const
{
  if( ptr_cb )
    ptr_cb(d, ev);
  else if( L != NULL && ref_cb != LUA_NOREF )
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref_cb);
    // callback type has already been checked in constructor
    // push the event
    lua_newtable(L);
    LuaManager::setfield(L, -1, "t", ev->type);
    switch( ev->type )
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        LuaManager::setfield(L, -1, "state", ev->key.state == SDL_PRESSED);
        LuaManager::setfield(L, -1, "key", ev->key.keysym.sym);
        lua_newtable(L);
        if( ev->key.keysym.mod != KMOD_NONE )
        {
          SDLMod mod = ev->key.keysym.mod;
          if( mod & KMOD_LSHIFT ) LuaManager::setfield(L, -1, "LSHIFT", true);
          if( mod & KMOD_RSHIFT ) LuaManager::setfield(L, -1, "RSHIFT", true);
          if( mod & KMOD_LCTRL  ) LuaManager::setfield(L, -1, "LCTRL" , true);
          if( mod & KMOD_RCTRL  ) LuaManager::setfield(L, -1, "RCTRL" , true);
          if( mod & KMOD_LALT   ) LuaManager::setfield(L, -1, "LALT"  , true);
          if( mod & KMOD_RALT   ) LuaManager::setfield(L, -1, "RALT"  , true);
          if( mod & KMOD_LMETA  ) LuaManager::setfield(L, -1, "LMETA" , true);
          if( mod & KMOD_RMETA  ) LuaManager::setfield(L, -1, "RMETA" , true);
          if( mod & KMOD_NUM    ) LuaManager::setfield(L, -1, "NUM"   , true);
          if( mod & KMOD_CAPS   ) LuaManager::setfield(L, -1, "CAPS"  , true);
          if( mod & KMOD_MODE   ) LuaManager::setfield(L, -1, "MODE"  , true);
          if( mod & KMOD_SHIFT  ) LuaManager::setfield(L, -1, "SHIFT" , true);
          if( mod & KMOD_CTRL   ) LuaManager::setfield(L, -1, "CTRL"  , true);
          if( mod & KMOD_ALT    ) LuaManager::setfield(L, -1, "ALT"   , true);
          if( mod & KMOD_META   ) LuaManager::setfield(L, -1, "META"  , true);
        }
        lua_setfield(L, -2, "mod");
        break;

      case SDL_MOUSEMOTION:
        lua_newtable(L);
        if( ev->motion.state )
        {
          Uint8 state = ev->motion.state;
          if( state & SDL_BUTTON_LMASK ) LuaManager::setfield(L, -1, "LEFT",   true);
          if( state & SDL_BUTTON_MMASK ) LuaManager::setfield(L, -1, "MIDDLE", true);
          if( state & SDL_BUTTON_RMASK ) LuaManager::setfield(L, -1, "RIGHT",  true);
        }
        lua_setfield(L, -2, "state");
        LuaManager::setfield(L, -1, "x", ev->motion.x);
        LuaManager::setfield(L, -1, "y", ev->motion.y);
        LuaManager::setfield(L, -1, "xrel", ev->motion.xrel);
        LuaManager::setfield(L, -1, "yrel", ev->motion.yrel);
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        LuaManager::setfield(L, -1, "button", ev->button.button);
        LuaManager::setfield(L, -1, "state", ev->button.state == SDL_PRESSED);
        LuaManager::setfield(L, -1, "x", ev->motion.x);
        LuaManager::setfield(L, -1, "y", ev->motion.y);
        break;

      default:
        throw(Error("EventHandler: event type not supported: %d", ev->type));
    }

    LuaManager::pcall(L, 1, 0);
  }
  else
    throw(Error("EventHandler: attempt to process an handler without callback"));
}


void Display::getCameraPos(btVector3 &eye_pos, btVector3 &target_pos) const
{
  // ref positions depends on the other point.
  // Thus, fixed and mobile positions are retrieved first.

  if( camera_mode & CAM_EYE_FIXED )
  {
    eye_pos = camera_eye.cart;
  }
  else if( camera_mode & CAM_EYE_OBJECT )
  {
    // Convert offset in global coordinates and add it
    eye_pos = camera_eye.obj->getTrans() * camera_eye.cart;
  }

  if( camera_mode & CAM_TARGET_FIXED )
  {
    target_pos = camera_target.cart;
  }
  else if( camera_mode & CAM_TARGET_OBJECT )
  {
    // Convert offset in global coordinates and add it
    target_pos = camera_target.obj->getTrans() * camera_target.cart;
  }

  if( camera_mode & CAM_EYE_REL )
  {
    eye_pos = camera_eye.spheric;

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera_mode & CAM_TARGET_OBJECT )
      eye_pos = quatRotate(camera_target.obj->getTrans().getRotation(), eye_pos);

    eye_pos += target_pos;
  }
  else if( camera_mode & CAM_TARGET_REL )
  {
    target_pos = camera_target.spheric;

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera_mode & CAM_EYE_OBJECT )
      target_pos = quatRotate(camera_eye.obj->getTrans().getRotation(), target_pos);

    target_pos += eye_pos;
  }
}


void Display::setCameraMode(int mode)
{
  if( camera_mode == mode )
    return;

  if( (mode & CAM_EYE_REL) && (mode & CAM_TARGET_REL) )
    throw(Error("invalid camera mode"));

  // Set new coordinates
  // Try to keep the same (global) positions

  btVector3 eye_pos, target_pos;
  getCameraPos(eye_pos, target_pos);

  switch( mode & CAM_EYE_MASK )
  {
    case CAM_EYE_FIXED:
      camera_eye.cart = eye_pos;
      break;
    case CAM_EYE_REL:
      camera_eye.spheric = btSpheric3( eye_pos - target_pos );
      break;
    case CAM_EYE_OBJECT:
      if( camera_eye.obj == NULL )
        throw(Error("object must be set before setting an object camera mode"));
      camera_eye.cart = btVector3(0,0,0);
      break;
    default:
      throw(Error("invalid camera mode"));
  }

  switch( mode & CAM_TARGET_MASK )
  {
    case CAM_TARGET_FIXED:
      for( int i=0; i<3; i++ )
        camera_target.cart = target_pos;
      break;
    case CAM_TARGET_REL:
      camera_target.spheric = btSpheric3( target_pos - eye_pos );
      break;
    case CAM_TARGET_OBJECT:
      if( camera_target.obj == NULL )
        throw(Error("object must be set before setting an object camera mode"));
      camera_target.cart = btVector3(0,0,0);
      break;
    default:
      throw(Error("invalid camera mode"));
  }

  camera_mode = mode;
  LOG->trace("camera mode: %x < %x", mode&CAM_EYE_MASK, (mode&CAM_TARGET_MASK)>>8);
}


void Display::handlerResize(Display *d, const SDL_Event *event)
{
  d->resize(event->resize.w, event->resize.h);
}

void Display::handlerPause(Display *d, const SDL_Event *event)
{
  physics->togglePause();
}

void Display::handlerCamMouse(Display *d, const SDL_Event *event)
{
  float dx = event->motion.xrel * cfg->camera_mouse_coef;
  float dy = event->motion.yrel * cfg->camera_mouse_coef;
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.rotate( dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
  else if( d->camera_mode & CAM_TARGET_REL )
    d->camera_target.spheric.rotate( dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
}


void Display::handlerCamAhead(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
  {
    d->camera_eye.spheric.r -= cfg->camera_step_linear;
    if( d->camera_eye.spheric.r < 0 )
      d->camera_eye.spheric.r = 0;
  }
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = cfg->camera_step_linear;
    d->camera_eye.cart += dir;
  }
}

void Display::handlerCamBack(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.r += cfg->camera_step_linear;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = -cfg->camera_step_linear;
    d->camera_eye.cart += dir;
  }
}

void Display::handlerCamLeft(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.phi -= cfg->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = cfg->camera_step_linear;
    dir.theta = M_PI_2;
    dir.phi += M_PI_2;
    d->camera_eye.cart += dir;
  }
}

void Display::handlerCamRight(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.phi += cfg->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = -cfg->camera_step_linear;
    dir.theta = M_PI_2;
    dir.phi += M_PI_2;
    d->camera_eye.cart += dir;
  }
}

void Display::handlerCamUp(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.theta -= cfg->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = -cfg->camera_step_linear;
    dir.theta -= M_PI_2;
    d->camera_eye.cart += dir;
  }
}

void Display::handlerCamDown(Display *d, const SDL_Event *event)
{
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.theta += cfg->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode & CAM_TARGET_FIXED )
      dir = d->camera_target.cart - d->camera_eye.cart;
    else if( d->camera_mode & CAM_TARGET_REL )
      dir = d->camera_target.spheric;
    else if( d->camera_mode & CAM_TARGET_OBJECT )
      dir = d->camera_target.obj->getPos() - d->camera_eye.cart;
    dir.r = cfg->camera_step_linear;
    dir.theta -= M_PI_2;
    d->camera_eye.cart += dir;
  }
}


OSDLua::OSDLua(lua_State *L, int ref_obj): L(L), ref_obj(ref_obj), ref_text(LUA_NOREF)
{
  if( this->ref_obj == LUA_NOREF || this->ref_obj == LUA_REFNIL )
    throw(LuaError(L, "OSDLua: invalid object reference"));
}

OSDLua::~OSDLua()
{
  luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);
  luaL_unref(L, LUA_REGISTRYINDEX, ref_text);
}

const char *OSDLua::getText()
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "text");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  const char *s = lua_tostring(L, -1);
  if( s == NULL )
    throw(Error("OSDLua: invalid 'text' field value"));

  // Hold a reference to avoid returned string to be collected
  if( ref_text != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_text);
  ref_text = luaL_ref(L, LUA_REGISTRYINDEX);

  return s;
}

int OSDLua::getX()
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "x");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  if( !lua_isnumber(L, -1) )
    throw(Error("OSDLua: invalid 'x' field value"));
  int i = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return i;
}

int OSDLua::getY()
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "y");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  if( !lua_isnumber(L, -1) )
    throw(Error("OSDLua: invalid 'y' field value"));
  int i = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return i;
}

Color4 OSDLua::getColor()
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "color");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  Color4 c;
  if( LuaManager::tocolor(L, -1, c) != 0 )
    throw(Error("OSDLua: invalid 'color' field value"));
  lua_pop(L, 1);
  return c;
}



/** @brief Lua Display class
 *
 * The constructor is a factory and returns the singleton.
 */
class LuaDisplay: public LuaClass<Display>
{
  static int _ctor(lua_State *L)
  {
    if( display == NULL )
      display = new Display();
    store_ptr(L, display);
    return 0;
  }

  LUA_DEFINE_SET0(init, init);
  LUA_DEFINE_GET(is_initialized, isInitialized);
  LUA_DEFINE_SET1(save_screenshot, savePNGScreenshot, LARG_s);

  LUA_DEFINE_SET1(set_camera_mode, setCameraMode, LARG_i);

  static int set_camera_eye(lua_State *L)
  {
    toCameraPoint(L, 2, &get_ptr(L,1)->camera_eye);
    return 0;
  }

  static int set_camera_target(lua_State *L)
  {
    toCameraPoint(L, 2, &get_ptr(L,1)->camera_target);
    return 0;
  }

  LUA_DEFINE_GET(get_camera_mode,getCameraMode);
  
  static int get_camera_eye(lua_State *L)
  {
    lua_newtable(L);
    pushCameraPoint(L, &get_ptr(L,1)->camera_eye);
    return 1;
  }

  static int get_camera_target(lua_State *L)
  {
    lua_newtable(L);
    pushCameraPoint(L, &get_ptr(L,1)->camera_target);
    return 1;
  }


  static int set_handler(lua_State *L)
  {
    SmartPtr<Display> d = get_ptr(L,1);

    SDL_Event ev;
    luaL_checktype(L, 2, LUA_TTABLE);
    toEvent(L, 2, &ev);

    if( lua_isnoneornil(L, 3) )
      d->setHandler(ev, Display::EventHandler(NULL));
    else
    {
      luaL_checktype(L, 3, LUA_TFUNCTION);
      lua_pushvalue(L, 3);
      int ref_cb = luaL_ref(L, LUA_REGISTRYINDEX);
      d->setHandler(ev, Display::EventHandler(L, ref_cb));
    }
    return 0;
  }

protected:
  /** @brief Get camera point info from a table.
   *
   * Parsed fields are: x, y, z, r, theta, phi, obj.
   * Invalid values are silently ignored.
   *
   * @note Argument is assumed to be a table.
   */
  static void toCameraPoint(lua_State *L, int narg, Display::CameraPoint *camera_point)
  {
    lua_getfield(L, narg, "x");
    if( lua_isnumber(L, -1) )
      camera_point->cart[0] = scale(lua_tonumber(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, narg, "y");
    if( lua_isnumber(L, -1) )
      camera_point->cart[1] = scale(lua_tonumber(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, narg, "z");
    if( lua_isnumber(L, -1) )
      camera_point->cart[2] = scale(lua_tonumber(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, narg, "r");
    if( lua_isnumber(L, -1) )
      camera_point->spheric.r = scale(lua_tonumber(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, narg, "theta");
    if( lua_isnumber(L, -1) )
      camera_point->spheric.theta = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, narg, "phi");
    if( lua_isnumber(L, -1) )
      camera_point->spheric.phi = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, narg, "obj");
    if( !lua_isnil(L, -1) )
      camera_point->obj = LuaClass<Object>::get_ptr(L,-1);
    lua_pop(L, 1);
  }

  /** @brief Push camera point info to a table.
   *
   * Pushed fields are: x, y, z, r, theta, phi, obj to the table at the top of
   * the stack.
   *
   * @note Argument is assumed to be a table.
   */
  static void pushCameraPoint(lua_State *L, Display::CameraPoint *camera_point)
  {
    LuaManager::setfield(L, -1, "x",     unscale(camera_point->cart[0]));
    LuaManager::setfield(L, -1, "y",     unscale(camera_point->cart[1]));
    LuaManager::setfield(L, -1, "z",     unscale(camera_point->cart[2]));
    LuaManager::setfield(L, -1, "r",     unscale(camera_point->spheric.r));
    LuaManager::setfield(L, -1, "theta", camera_point->spheric.theta);
    LuaManager::setfield(L, -1, "phi",   camera_point->spheric.phi);
    LuaManager::setfield(L, -1, "obj",   camera_point->obj.get());
  }


  /** @brief Get SDL event from a table.
   *
   * @note Argument is assumed to be a table.
   */
  static void toEvent(lua_State *L, int narg, SDL_Event *ev)
  {
    lua_getfield(L, narg, "t");
    if( ! lua_isnumber(L, -1) )
      luaL_error(L, "invalid handler type");
    ev->type = lua_tointeger(L, -1);
    lua_pop(L,1);

    switch( ev->type )
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        lua_getfield(L, narg, "key");
        // key: single character string or integer
        if( lua_isstring(L, -1) && lua_objlen(L, -1) == 1 )
        {
          char key = *lua_tostring(L, -1);
          // use only lowercase letters
          if( key >= 'A' && key <= 'Z' ) key |= 0x20;
          ev->key.keysym.sym = (SDLKey)key;
        }
        else if( lua_isnumber(L, -1) )
          ev->key.keysym.sym = (SDLKey)lua_tointeger(L, -1);
        else
          luaL_error(L, "invalid handler 'key' value");
        lua_pop(L,1);
        break;

      case SDL_MOUSEMOTION:
        lua_getfield(L, narg, "state");
        if( lua_isnoneornil(L, -1) )
          ev->motion.state = 0;
        else if( lua_istable(L, -1) )
        {
          ev->motion.state = 0;
          lua_getfield(L, -1, "LEFT");
          if( lua_toboolean(L, -1) ) ev->motion.state |= SDL_BUTTON_LMASK;
          lua_pop(L, 1);
          lua_getfield(L, -1, "MIDDLE");
          if( lua_toboolean(L, -1) ) ev->motion.state |= SDL_BUTTON_MMASK;
          lua_pop(L, 1);
          lua_getfield(L, -1, "RIGHT");
          if( lua_toboolean(L, -1) ) ev->motion.state |= SDL_BUTTON_RMASK;
          lua_pop(L, 1);
        }
        else
          luaL_error(L, "invalid handler 'state' value");
        lua_pop(L,1);
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        lua_getfield(L, narg, "button");
        if( lua_isnumber(L, -1) )
          ev->button.button = lua_tointeger(L, -1);
        else
          luaL_error(L, "invalid handler 'button' value");
        lua_pop(L,1);
        break;

      default:
        luaL_error(L, "invalid handler type");
    }
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(init);
    LUA_CLASS_MEMBER(is_initialized);
    LUA_CLASS_MEMBER(save_screenshot);
    LUA_CLASS_MEMBER(set_camera_mode);
    LUA_CLASS_MEMBER(set_camera_eye);
    LUA_CLASS_MEMBER(set_camera_target);
    LUA_CLASS_MEMBER(get_camera_mode);
    LUA_CLASS_MEMBER(get_camera_eye);
    LUA_CLASS_MEMBER(get_camera_target);
    LUA_CLASS_MEMBER(set_handler);

    // Camera modes
    lua_newtable(L);
#define SET_CAM_ENUM(N)  LuaManager::setfield(L, -1, #N, Display::CAM_##N)
      SET_CAM_ENUM(EYE_FIXED);
      SET_CAM_ENUM(EYE_FIXED);
      SET_CAM_ENUM(EYE_REL);
      SET_CAM_ENUM(EYE_OBJECT);
      SET_CAM_ENUM(EYE_MASK);
      SET_CAM_ENUM(TARGET_FIXED);
      SET_CAM_ENUM(TARGET_REL);
      SET_CAM_ENUM(TARGET_OBJECT);
      SET_CAM_ENUM(TARGET_MASK);
      SET_CAM_ENUM(FREE);
      SET_CAM_ENUM(FIXED);
      SET_CAM_ENUM(FOLLOW);
      SET_CAM_ENUM(ONBOARD);
      SET_CAM_ENUM(LOOK);
#undef SET_CAM_ENUM
    lua_setfield(L, -2, "CAM");

    // Event handler types
    lua_newtable(L);
#define SET_EVENT_ENUM(N)  LuaManager::setfield(L, -1, #N, SDL_##N)
      SET_EVENT_ENUM(KEYDOWN);
      SET_EVENT_ENUM(KEYUP);
      SET_EVENT_ENUM(MOUSEMOTION);
      SET_EVENT_ENUM(MOUSEBUTTONDOWN);
      SET_EVENT_ENUM(MOUSEBUTTONUP);
#undef SET_EVENT_ENUM
    lua_setfield(L, -2, "EVENT");

    // Keys
    lua_newtable(L);
#define SET_KEY_ENUM(N)  LuaManager::setfield(L, -1, #N, SDLK_##N)
      SET_KEY_ENUM(BACKSPACE);
      SET_KEY_ENUM(TAB);
      SET_KEY_ENUM(CLEAR);
      SET_KEY_ENUM(RETURN);
      SET_KEY_ENUM(PAUSE);
      SET_KEY_ENUM(ESCAPE);
      SET_KEY_ENUM(DELETE);
	    SET_KEY_ENUM(KP0);
	    SET_KEY_ENUM(KP1);
	    SET_KEY_ENUM(KP2);
	    SET_KEY_ENUM(KP3);
	    SET_KEY_ENUM(KP4);
	    SET_KEY_ENUM(KP5);
	    SET_KEY_ENUM(KP6);
	    SET_KEY_ENUM(KP7);
	    SET_KEY_ENUM(KP8);
	    SET_KEY_ENUM(KP9);
	    SET_KEY_ENUM(KP_PERIOD);
	    SET_KEY_ENUM(KP_DIVIDE);
	    SET_KEY_ENUM(KP_MULTIPLY);
	    SET_KEY_ENUM(KP_MINUS);
	    SET_KEY_ENUM(KP_PLUS);
	    SET_KEY_ENUM(KP_ENTER);
	    SET_KEY_ENUM(KP_EQUALS);
	    SET_KEY_ENUM(UP);
	    SET_KEY_ENUM(DOWN);
	    SET_KEY_ENUM(RIGHT);
	    SET_KEY_ENUM(LEFT);
	    SET_KEY_ENUM(INSERT);
	    SET_KEY_ENUM(HOME);
	    SET_KEY_ENUM(END);
	    SET_KEY_ENUM(PAGEUP);
	    SET_KEY_ENUM(PAGEDOWN);
	    SET_KEY_ENUM(F1);
	    SET_KEY_ENUM(F2);
	    SET_KEY_ENUM(F3);
	    SET_KEY_ENUM(F4);
	    SET_KEY_ENUM(F5);
	    SET_KEY_ENUM(F6);
	    SET_KEY_ENUM(F7);
	    SET_KEY_ENUM(F8);
	    SET_KEY_ENUM(F9);
	    SET_KEY_ENUM(F10);
	    SET_KEY_ENUM(F11);
	    SET_KEY_ENUM(F12);
	    SET_KEY_ENUM(F13);
	    SET_KEY_ENUM(F14);
	    SET_KEY_ENUM(F15);
	    SET_KEY_ENUM(NUMLOCK);
	    SET_KEY_ENUM(CAPSLOCK);
	    SET_KEY_ENUM(SCROLLOCK);
	    SET_KEY_ENUM(RSHIFT);
	    SET_KEY_ENUM(LSHIFT);
	    SET_KEY_ENUM(RCTRL);
	    SET_KEY_ENUM(LCTRL);
	    SET_KEY_ENUM(RALT);
	    SET_KEY_ENUM(LALT);
	    SET_KEY_ENUM(RMETA);
	    SET_KEY_ENUM(LMETA);
	    SET_KEY_ENUM(LSUPER);
	    SET_KEY_ENUM(RSUPER);
	    SET_KEY_ENUM(MODE);
	    SET_KEY_ENUM(COMPOSE);
	    SET_KEY_ENUM(HELP);
	    SET_KEY_ENUM(PRINT);
	    SET_KEY_ENUM(SYSREQ);
	    SET_KEY_ENUM(BREAK);
	    SET_KEY_ENUM(MENU);
	    SET_KEY_ENUM(POWER);
	    SET_KEY_ENUM(EURO);
	    SET_KEY_ENUM(UNDO);
#undef SET_KEY_ENUM
    lua_setfield(L, -2, "KEY");

    // Mouse buttons
    lua_newtable(L);
#define SET_BUTTON_ENUM(N)  LuaManager::setfield(L, -1, #N, SDL_BUTTON_##N)
      SET_BUTTON_ENUM(LEFT);
      SET_BUTTON_ENUM(MIDDLE);
      SET_BUTTON_ENUM(RIGHT);
      SET_BUTTON_ENUM(WHEELUP);
      SET_BUTTON_ENUM(WHEELDOWN);
#undef SET_BUTTON_ENUM
    lua_setfield(L, -2, "BUTTON");

  }
};

LUA_REGISTER_BASE_CLASS(Display);


/** @brief Lua OSD class
 */
class LuaOSD: public LuaClass<OSDLua>
{
  static int _ctor(lua_State *L)
  {
    data_ptr *ud = new_ptr(L);
    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    *ud = new OSDLua(L, ref);
    SmartPtr_add_ref(*ud);
    return 0;
  }

  static int show(lua_State *L)
  {
    if( !display )
      throw(LuaError(L, "no display"));
    SmartPtr<OSDMessage> osd = get_ptr(L,1);
    display->getOsds().insert( osd );
    return 0;
  }
  static int hide(lua_State *L)
  {
    if( !display )
      throw(LuaError(L, "no display"));
    display->getOsds().erase( SmartPtr<OSDMessage>(get_ptr(L,1)) );
    return 0;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(show);
    LUA_CLASS_MEMBER(hide);
  }
};


LUA_REGISTER_BASE_CLASS_NAME(LuaOSD,OSDLua,"OSD");

