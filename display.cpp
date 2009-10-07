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

  event.key.keysym.sym = SDLK_c;
  event.type = SDL_KEYUP;
  setHandler(event, handlerCamMode);

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



void Display::handleEvents()
{
  SDL_Event event;
  std::map<SDL_Event, EventHandler, EventCmp>::iterator ithandler;

  while(SDL_PollEvent(&event))
  {
    ithandler = handlers.find(event);
    if( ithandler != handlers.end() )
      (*ithandler).second(this, event);
  }
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
        throw(Error("object must be set before setting an object camera mode."));
      camera_target.cart = btVector3(0,0,0);
      break;
    default:
      throw(Error("invalid camera mode"));
  }

  camera_mode = mode;
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


void Display::handlerResize(Display *d, const SDL_Event &event)
{
  d->resize(event.resize.w, event.resize.h);
}

void Display::handlerPause(Display *d, const SDL_Event &event)
{
  physics->togglePause();
}

void Display::handlerCamMode(Display *d, const SDL_Event &event)
{
  switch( d->getCameraMode() )
  {
    case CAM_FIXED:
      d->setCameraMode(CAM_FREE);
      break;
    case CAM_FREE:
      {
        /*TODO
        d->camera_target.obj = (*robots.begin()).second;
        d->setCameraMode(CAM_LOOK);
        break;
        */
      }
    case CAM_LOOK:
      {
        /*TODO
        d->camera_eye.obj = (*robots.begin()).second;
        d->setCameraMode(CAM_ONBOARD);
        d->camera_target.spheric = btSpheric3(1.0, 3*M_PI/4, 0.0);//XXX
        d->camera_eye.cart = scale(btVector3(0.0, 0.0, 0.3));//XXX
        break;
        */
      }
    case CAM_ONBOARD:
    default:
      d->setCameraMode(CAM_FIXED);
      break;
  }
}

void Display::handlerCamMouse(Display *d, const SDL_Event &event)
{
  float dx = event.motion.xrel * cfg->camera_mouse_coef;
  float dy = event.motion.yrel * cfg->camera_mouse_coef;
  if( d->camera_mode & CAM_EYE_REL )
    d->camera_eye.spheric.rotate( dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
  else if( d->camera_mode & CAM_TARGET_REL )
    d->camera_target.spheric.rotate( dy*cfg->camera_step_angle, -dx*cfg->camera_step_angle );
}


void Display::handlerCamAhead(Display *d, const SDL_Event &event)
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

void Display::handlerCamBack(Display *d, const SDL_Event &event)
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

void Display::handlerCamLeft(Display *d, const SDL_Event &event)
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

void Display::handlerCamRight(Display *d, const SDL_Event &event)
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

void Display::handlerCamUp(Display *d, const SDL_Event &event)
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

void Display::handlerCamDown(Display *d, const SDL_Event &event)
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



OSDLua::OSDLua(int ref_obj): ref_obj(ref_obj), ref_text(LUA_NOREF)
{
  if( this->ref_obj == LUA_NOREF || this->ref_obj == LUA_REFNIL )
    throw(LuaError("invalid object reference for OSDLua"));
}

OSDLua::~OSDLua()
{
  lua_State *L = lm->get_L();
  luaL_unref(L, LUA_REGISTRYINDEX, ref_obj);
  luaL_unref(L, LUA_REGISTRYINDEX, ref_text);
}

const char *OSDLua::getText()
{
  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "text");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  const char *s = lua_tostring(L, -1);
  if( s == NULL )
    throw(LuaError("OSDLua: invalid 'text' field value"));

  // Hold a reference to avoid returned string to be collected
  if( ref_text != LUA_NOREF )
    luaL_unref(L, LUA_REGISTRYINDEX, ref_text);
  ref_text = luaL_ref(L, LUA_REGISTRYINDEX);

  return s;
}

int OSDLua::getX()
{
  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "x");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  if( !lua_isnumber(L, -1) )
    throw(LuaError("OSDLua: invalid 'x' field value"));
  int i = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return i;
}

int OSDLua::getY()
{
  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "y");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  if( !lua_isnumber(L, -1) )
    throw(LuaError("OSDLua: invalid 'y' field value"));
  int i = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return i;
}

Color4 OSDLua::getColor()
{
  lua_State *L = lm->get_L();
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref_obj);
  lua_remove(L, -2);
  lua_getfield(L, -1, "color");
  if( lua_isfunction(L, -1) )
    LuaManager::pcall(L, 0, 1);
  Color4 c;
  if( LuaManager::tocolor(L, -1, c) != 0 )
    throw(LuaError("OSDLua: invalid 'color' field value"));
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

  LUA_DEFINE_SET1(set_camera_mode, setCameraMode, LARG_n);

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
    lua_pushnumber(L, unscale(camera_point->cart[0]));
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, unscale(camera_point->cart[1]));
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, unscale(camera_point->cart[2]));
    lua_setfield(L, -2, "z");
    lua_pushnumber(L, unscale(camera_point->spheric.r));
    lua_setfield(L, -2, "r");
    lua_pushnumber(L, unscale(camera_point->spheric.theta));
    lua_setfield(L, -2, "theta");
    lua_pushnumber(L, unscale(camera_point->spheric.phi));
    lua_setfield(L, -2, "phi");
    LuaManager::push(L, camera_point->obj.get());
    lua_setfield(L, -2, "obj");
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
    // Camera modes
    LUA_CLASS_MEMBER_VAL("CAM_EYE_FIXED",Display::CAM_EYE_FIXED);
    LUA_CLASS_MEMBER_VAL("CAM_EYE_REL",Display::CAM_EYE_REL);
    LUA_CLASS_MEMBER_VAL("CAM_EYE_OBJECT",Display::CAM_EYE_OBJECT);
    LUA_CLASS_MEMBER_VAL("CAM_EYE_MASK",Display::CAM_EYE_MASK);
    LUA_CLASS_MEMBER_VAL("CAM_TARGET_FIXED",Display::CAM_TARGET_FIXED);
    LUA_CLASS_MEMBER_VAL("CAM_TARGET_REL",Display::CAM_TARGET_REL);
    LUA_CLASS_MEMBER_VAL("CAM_TARGET_OBJECT",Display::CAM_TARGET_OBJECT);
    LUA_CLASS_MEMBER_VAL("CAM_TARGET_MASK",Display::CAM_TARGET_MASK);
    LUA_CLASS_MEMBER_VAL("CAM_FREE",Display::CAM_FREE);
    LUA_CLASS_MEMBER_VAL("CAM_FIXED",Display::CAM_FIXED);
    LUA_CLASS_MEMBER_VAL("CAM_FOLLOW",Display::CAM_FOLLOW);
    LUA_CLASS_MEMBER_VAL("CAM_ONBOARD",Display::CAM_ONBOARD);
    LUA_CLASS_MEMBER_VAL("CAM_LOOK",Display::CAM_LOOK);
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
    *ud = new OSDLua(ref);
    SmartPtr_add_ref(*ud);
    return 0;
  }

  static int show(lua_State *L)
  {
    if( !display )
      throw(LuaError("OSD:show(): no display"));
    SmartPtr<OSDMessage> osd = get_ptr(L,1);
    display->getOsds().insert( osd );
    return 0;
  }
  static int hide(lua_State *L)
  {
    if( !display )
      throw(LuaError("OSD:hide(): no display"));
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

