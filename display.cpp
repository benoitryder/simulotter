#include <cmath>
#include <cstring>
#include <cstdio>
#include <SDL/SDL_opengl.h>
#include <png.h>
#include "display.h"
#include "physics.h"
#include "object.h"
#include "log.h"
#include "icon.h"



btScalar Display::draw_epsilon = btScale(0.0005);
unsigned int Display::draw_div = 20;
unsigned int Display::antialias = 0;


Display::Display():
    time_scale(1.0), fps(60.0),
    bg_color(Color4(0.8)),
    camera_step_angle(0.1),
    camera_step_linear(btScale(0.1)),
    camera_mouse_coef(0.1),
    perspective_fov(45.0),
    perspective_near(btScale(0.1)),
    perspective_far(btScale(300.0)),
    screen_x_(800), screen_y_(600),
    fullscreen_(false)
{
  int argc = 1;
  glutInit(&argc, NULL);

  screen_ = NULL;

  camera_mode_ = CAM_FIXED;
  camera_eye_.spheric = btScale( btSpheric3(4.0, M_PI/6, -M_PI/3) );
  camera_target_.cart = btVector3(0,0,0);
  camera_eye_.obj    = NULL;
  camera_target_.obj = NULL;

  // Default handlers

  SDL_Event event;
  
  // Window events
  event.type = SDL_QUIT;
  setHandler(event, new BasicEventHandler(handlerQuit));
  event.type = SDL_VIDEORESIZE;
  setHandler(event, new BasicEventHandler(handlerResize));

  // Mouse motion
  event.type = SDL_MOUSEMOTION;
  event.motion.state = SDL_BUTTON(1);
  setHandler(event, new BasicEventHandler(handlerCamMouse));

  // Keyboard

  event.key.keysym.sym = SDLK_ESCAPE;
  event.type = SDL_KEYDOWN;
  setHandler(event, new BasicEventHandler(handlerQuit));

  event.key.keysym.sym = SDLK_SPACE;
  event.type = SDL_KEYUP;
  setHandler(event, new BasicEventHandler(handlerPause));

  // Camera moves
  // TODO correct AZERTY/QWERTY handling
  event.type = SDL_KEYDOWN;
#ifdef WIN32
  event.key.keysym.sym = SDLK_w;
#else
  event.key.keysym.sym = SDLK_z;
#endif
  setHandler(event, new BasicEventHandler(handlerCamAhead));
  event.key.keysym.sym = SDLK_s;
  setHandler(event, new BasicEventHandler(handlerCamBack));
#ifdef WIN32
  event.key.keysym.sym = SDLK_a;
#else
  event.key.keysym.sym = SDLK_q;
#endif
  setHandler(event, new BasicEventHandler(handlerCamLeft));
  event.key.keysym.sym = SDLK_d;
  setHandler(event, new BasicEventHandler(handlerCamRight));
#ifdef WIN32
  event.key.keysym.sym = SDLK_q;
#else
  event.key.keysym.sym = SDLK_a;
#endif
  setHandler(event, new BasicEventHandler(handlerCamUp));
  event.key.keysym.sym = SDLK_e;
  setHandler(event, new BasicEventHandler(handlerCamDown));
}

Display::~Display()
{
  windowDestroy();
  sceneDestroy();
}


void Display::resize(int width, int height, int mode)
{
  if( !windowInitialized() ) {
    windowInit();
  }

  bool fullscreen;
  if( mode == 0 )
    fullscreen = false;
  else if( mode > 0 )
    fullscreen = true;
  else
    fullscreen = fullscreen_;

  LOG("Display resize: %d x %d, %s mode",
      width, height, fullscreen ? "fullscreen" : "window");

  Uint32 flags = SDL_OPENGL;
  flags |= fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE;

  // Delete display lists.
  // On Windows setting the video mode resets the current OpenGL context.
  // We always reset display lists, it's safer.
  DisplayListContainer::const_iterator it;
  for( it=display_lists_.begin(); it!=display_lists_.end(); ++it ) {
    glDeleteLists((*it).second, 1);
  }
  display_lists_.clear();

  if( (screen_ = SDL_SetVideoMode(width, height, 0, flags)) == NULL )
  {
    windowDestroy();
    throw(Error("SDL: cannot change video mode"));
  }

  screen_x_ = width;
  screen_y_ = height;
  fullscreen_ = fullscreen;

  sceneInit();
}


void Display::update()
{
  if( ! physics_ )
    throw(Error("no physics attached to display"));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity();
  gluPerspective(this->perspective_fov, ((float)screen_x_)/screen_y_,
      this->perspective_near, this->perspective_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  btVector3 eye_pos, target_pos;
  getCameraPos(eye_pos, target_pos);

  float up = (
      ( (camera_mode_ & CAM_EYE_REL) &&
        ((int)btCeil(camera_eye_.spheric.theta/M_PI))%2==0 ) ||
      ( (camera_mode_ & CAM_TARGET_REL) &&
        ((int)btCeil(camera_target_.spheric.theta/M_PI))%2==0 )
      ) ? -1.0 : 1.0;

  gluLookAt(
      eye_pos[0], eye_pos[1], eye_pos[2],
      target_pos[0], target_pos[1], target_pos[2],
      0.0, 0.0, up);

  // Draw objects
  const std::set< SmartPtr<Object> > &objs = physics_->getObjs();
  std::set< SmartPtr<Object> >::const_iterator it_obj;
  for( it_obj = objs.begin(); it_obj != objs.end(); ++it_obj )
    (*it_obj)->draw(this);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, screen_x_, 0.0, screen_y_);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // OSD
  glDisable(GL_LIGHTING);
  std::set< SmartPtr<OSDMessage> >::iterator it_osd;
  for( it_osd = osds_.begin(); it_osd != osds_.end(); ++it_osd )
    drawString( (*it_osd)->getText(),
        (*it_osd)->getX(), (*it_osd)->getY(),
        (*it_osd)->getColor(), GLUT_BITMAP_8_BY_13
        );
  glEnable(GL_LIGHTING);

  SDL_GL_SwapBuffers();
}


void Display::run()
{
  if( ! physics_ ) {
    throw(Error("no physics attached to display"));
  }
  // window not created yet, force resize() to create it
  if( ! windowInitialized() ) {
    resize(screen_x_, screen_y_, fullscreen_);
  }

  unsigned int step_dt = (unsigned int)(1000.0*physics_->getStepDt());
  unsigned long time;
  unsigned long time_disp, time_step;
  signed long time_wait;

  time_disp = time_step = SDL_GetTicks();
  for(;;) {
    time = SDL_GetTicks();
    if( time >= time_step ) {
      physics_->step();
      time_step += (unsigned long)(step_dt * this->time_scale);
    }
    if( time >= time_disp ) {
      this->processEvents();
      this->update();
      time_disp = time + (1000.0/this->fps);
    }

    time_wait = MIN(time_step,time_disp);
    time_wait -= time;
    if( time_wait > 0 )
      SDL_Delay(time_wait);
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

static void png_handler_warning(png_struct * /*png_ptr*/, const char *msg)
{
  LOG("PNG: warning: %s", msg);
}

//@}

void Display::savePNGScreenshot(const std::string &filename)
{
  FILE *fp = NULL;
  png_bytep pixels = NULL;
  png_bytepp row_pointers = NULL;

  try
  {
    // Open output file
    fp = fopen(filename.c_str(), "wb");
    if( !fp )
      throw(Error("cannot open file '%s' for writing", filename.c_str()));

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

    png_set_IHDR(png_ptr, info_ptr, screen_x_, screen_y_, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    // Get pixels, create pixel rows
    glFlush();
    SDL_LockSurface(screen_);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    SDL_UnlockSurface(screen_);
    pixels = new unsigned char[4*screen_x_*screen_y_];
    glReadPixels(0, 0, screen_x_, screen_y_, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    row_pointers = new png_bytep[screen_y_];
    int i;
    for( i=0; i<screen_y_; i++ )
      row_pointers[screen_y_-i-1] = (png_bytep)pixels+3*i*screen_x_;
    png_set_rows(png_ptr, info_ptr, row_pointers);

    // Write data
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    delete[] row_pointers; row_pointers = NULL;
    delete[] pixels; pixels = NULL;
    fclose(fp); fp = NULL;

    LOG("screenshot saved: %s", filename.c_str());
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


void Display::drawString(const std::string &s, int x, int y, Color4 color, void *font)
{
  y = screen_y_ - y;
  glColor4fv(color);
  glRasterPos2f(x,y);
  for( unsigned int i=0; i<s.size(); i++)
    glutBitmapCharacter(font, s[i]);
}


void Display::windowInit()
{
  if( windowInitialized() ) {
    throw(Error("window already initialized")); // should not happen
  }

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

  if( antialias > 0 ) {
    if( SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) < 0 )
      throw(Error("SDL: cannot enable multisample buffers"));
    if( SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialias) < 0 )
      throw(Error("SDL: cannot set multisample sample count to %d", antialias));
  }
}

void Display::windowDestroy()
{
  SDL_Quit();
}


void Display::sceneInit()
{
  // Clear the background color
  glClearColor(this->bg_color[0], this->bg_color[1],
      this->bg_color[2], this->bg_color[3]);
  glViewport(0, 0, screen_x_, screen_y_);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  const GLfloat light_pos[4] = {0,btScale(.3),btScale(.8),0};
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


bool Display::callOrCreateDisplayList(void *key)
{
  DisplayListContainer::const_iterator it = display_lists_.find(key);
  if( it != display_lists_.end() ) {
    glCallList( (*it).second );
    return false;
  }

  GLuint id = glGenLists(1);
  if( id == 0 ) {
    throw(Error("failed to create display list"));
  }
  display_lists_[key] = id;
  glNewList(id, GL_COMPILE_AND_EXECUTE);

  return true;
}

void Display::endDisplayList()
{
  glEndList();
}



void Display::processEvents()
{
  SDL_Event event;
  EventHandlerContainer::iterator it_h;

  while(SDL_PollEvent(&event))
  {
    it_h = handlers_.find(event);
    if( it_h != handlers_.end() )
      (*it_h).second->process(this, &event);
  }
}

void Display::setHandler(const SDL_Event &ev, DisplayEventHandler *h)
{
  if( h == NULL ) {
    handlers_.erase(ev);
  } else {
    handlers_.insert( EventHandlerContainer::value_type(ev, h) );
  }
}


void Display::getCameraPos(btVector3 &eye_pos, btVector3 &target_pos) const
{
  // ref positions depends on the other point.
  // Thus, fixed and mobile positions are retrieved first.

  if( camera_mode_ & CAM_EYE_FIXED )
  {
    eye_pos = camera_eye_.cart;
  }
  else if( camera_mode_ & CAM_EYE_OBJECT )
  {
    // Convert offset in global coordinates and add it
    eye_pos = camera_eye_.obj->getTrans() * camera_eye_.cart;
  }

  if( camera_mode_ & CAM_TARGET_FIXED )
  {
    target_pos = camera_target_.cart;
  }
  else if( camera_mode_ & CAM_TARGET_OBJECT )
  {
    // Convert offset in global coordinates and add it
    target_pos = camera_target_.obj->getTrans() * camera_target_.cart;
  }

  if( camera_mode_ & CAM_EYE_REL )
  {
    eye_pos = camera_eye_.spheric;

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera_mode_ & CAM_TARGET_OBJECT )
      eye_pos = quatRotate(camera_target_.obj->getTrans().getRotation(), eye_pos);

    eye_pos += target_pos;
  }
  else if( camera_mode_ & CAM_TARGET_REL )
  {
    target_pos = camera_target_.spheric;

    // Object relative coordinates are in object coordinates
    // Convert them to global coordinates
    if( camera_mode_ & CAM_EYE_OBJECT )
      target_pos = quatRotate(camera_eye_.obj->getTrans().getRotation(), target_pos);

    target_pos += eye_pos;
  }
}


void Display::setCameraMode(int mode)
{
  if( camera_mode_ == mode )
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
      camera_eye_.cart = eye_pos;
      break;
    case CAM_EYE_REL:
      camera_eye_.spheric = btSpheric3( eye_pos - target_pos );
      break;
    case CAM_EYE_OBJECT:
      if( camera_eye_.obj == NULL )
        throw(Error("object must be set before setting an object camera mode"));
      camera_eye_.cart = btVector3(0,0,0);
      break;
    default:
      throw(Error("invalid camera mode"));
  }

  switch( mode & CAM_TARGET_MASK )
  {
    case CAM_TARGET_FIXED:
      for( int i=0; i<3; i++ )
        camera_target_.cart = target_pos;
      break;
    case CAM_TARGET_REL:
      camera_target_.spheric = btSpheric3( target_pos - eye_pos );
      break;
    case CAM_TARGET_OBJECT:
      if( camera_target_.obj == NULL )
        throw(Error("object must be set before setting an object camera mode"));
      camera_target_.cart = btVector3(0,0,0);
      break;
    default:
      throw(Error("invalid camera mode"));
  }

  camera_mode_ = mode;
  LOG("camera mode: %x < %x", mode&CAM_EYE_MASK, (mode&CAM_TARGET_MASK)>>8);
}


void Display::handlerResize(Display *d, const SDL_Event *event)
{
  d->resize(event->resize.w, event->resize.h);
}

void Display::handlerPause(Display *d, const SDL_Event *event)
{
  Physics *ph = d->getPhysics();
  if( ! ph )
    throw(Error("no physics attached to display"));
  ph->togglePause();
}

void Display::handlerCamMouse(Display *d, const SDL_Event *event)
{
  float dx = event->motion.xrel * d->camera_mouse_coef;
  float dy = event->motion.yrel * d->camera_mouse_coef;
  if( d->camera_mode_ & CAM_EYE_REL ) {
    d->camera_eye_.spheric.rotate( dy*d->camera_step_angle,
                                  -dx*d->camera_step_angle );
  } else if( d->camera_mode_ & CAM_TARGET_REL ) {
    d->camera_target_.spheric.rotate( dy*d->camera_step_angle,
                                     -dx*d->camera_step_angle );
  }
}


void Display::handlerCamAhead(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
  {
    d->camera_eye_.spheric.r -= d->camera_step_linear;
    if( d->camera_eye_.spheric.r < 0 )
      d->camera_eye_.spheric.r = 0;
  }
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = d->camera_step_linear;
    d->camera_eye_.cart += dir;
  }
}

void Display::handlerCamBack(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
    d->camera_eye_.spheric.r += d->camera_step_linear;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = -d->camera_step_linear;
    d->camera_eye_.cart += dir;
  }
}

void Display::handlerCamLeft(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
    d->camera_eye_.spheric.phi -= d->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = d->camera_step_linear;
    dir.theta = M_PI_2;
    dir.phi += M_PI_2;
    d->camera_eye_.cart += dir;
  }
}

void Display::handlerCamRight(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
    d->camera_eye_.spheric.phi += d->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = -d->camera_step_linear;
    dir.theta = M_PI_2;
    dir.phi += M_PI_2;
    d->camera_eye_.cart += dir;
  }
}

void Display::handlerCamUp(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
    d->camera_eye_.spheric.theta -= d->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = -d->camera_step_linear;
    dir.theta -= M_PI_2;
    d->camera_eye_.cart += dir;
  }
}

void Display::handlerCamDown(Display *d, const SDL_Event *event)
{
  if( d->camera_mode_ & CAM_EYE_REL )
    d->camera_eye_.spheric.theta += d->camera_step_angle;
  else
  {
    btSpheric3 dir;
    if( d->camera_mode_ & CAM_TARGET_FIXED )
      dir = d->camera_target_.cart - d->camera_eye_.cart;
    else if( d->camera_mode_ & CAM_TARGET_REL )
      dir = d->camera_target_.spheric;
    else if( d->camera_mode_ & CAM_TARGET_OBJECT )
      dir = d->camera_target_.obj->getPos() - d->camera_eye_.cart;
    dir.r = d->camera_step_linear;
    dir.theta -= M_PI_2;
    d->camera_eye_.cart += dir;
  }
}


bool DisplayEventHandler::Cmp::operator()(const SDL_Event &a, const SDL_Event &b)
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


