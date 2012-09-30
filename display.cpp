#include <cmath>
#include <cstring>
#include <cstdio>
#include <exception>
#include <memory>
#include <SDL/SDL_opengl.h>
#include <png.h>
#include "display.h"
#include "physics.h"
#include "object.h"
#include "log.h"
#include "icon.h"


btScalar Display::draw_epsilon = 0.0005_m;
unsigned int Display::draw_div = 20;
unsigned int Display::antialias = 0;


Display::Display():
  time_scale_(1.0), fps_(60.0),
  paused_(false),
  bg_color_(Color4(0.8)),
  camera_step_linear_(0.1_m),
  camera_mouse_coef_(0.01),
  screen_x_(800), screen_y_(600),
  fullscreen_(false),
  is_running_(false)
{
  int argc = 1;
  glutInit(&argc, NULL);

  screen_ = NULL;

  handlerCamReset(this, NULL);
  setDefaultHandlers();
}

Display::~Display()
{
  sceneDestroy();
  windowDestroy();
}


void Display::resize(int width, int height, int mode)
{
  if(!windowInitialized()) {
    windowInit();
  }

  bool fullscreen;
  if(mode == 0) {
    fullscreen = false;
  } else if(mode > 0) {
    fullscreen = true;
  } else {
    fullscreen = fullscreen_;
  }

  Uint32 flags = SDL_OPENGL;
  flags |= fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE;

  // Delete display lists.
  // On Windows setting the video mode resets the current OpenGL context.
  // We always reset display lists, it's safer.
  for(auto& it : display_lists_) {
    glDeleteLists(it.second, 1);
  }
  display_lists_.clear();

  if(!(screen_ = SDL_SetVideoMode(width, height, 0, flags))) {
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
  if(!physics_) {
    throw(Error("no physics attached to display"));
  }
  // window not created yet, force resize() to create it
  if(!windowInitialized()) {
    resize(screen_x_, screen_y_, fullscreen_);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(camera_.fov, ((float)screen_x_)/screen_y_,
      camera_.z_near, camera_.z_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  {
    btScalar m[16];
    btTransform tr;
    if(!camera_.obj) {
      tr = camera_.trans;
    } else {
      tr = camera_.obj->getTrans() * camera_.trans;
    }
    tr.getOpenGLMatrix(m);
    m[12] = m[13] = m[14] = 0;  // no translation yet
    btglMultMatrix(m);
    const btVector3& v = tr.getOrigin();
    btglTranslate(-v.x(), -v.y(), -v.z());
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Draw objects
  const std::set<SmartPtr<Object>>& objs = physics_->getObjs();
  for(auto& obj : objs) {
    obj->draw(this);
  }
  for(auto& obj : objs) {
    obj->drawLast(this);
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, screen_x_, 0.0, screen_y_);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // OSD
  glDisable(GL_LIGHTING);
  for(auto& osd : osds_) {
    drawString(osd->getText(), osd->getX(), osd->getY(), osd->getColor(), GLUT_BITMAP_8_BY_13);
  }
  glEnable(GL_LIGHTING);

  SDL_GL_SwapBuffers();
}

void Display::close()
{
  if(!windowInitialized()) {
    throw(Error("display window not opened"));
  }
  windowDestroy();
  if(is_running_) {
    abort();
  }
}


Display::Camera::Camera():
    trans(btTransform::getIdentity()),
    fov(45.0), z_near(0.1_m), z_far(300.0_m)
{
}

void Display::Camera::mouseMove(btScalar x, btScalar y)
{
  const btMatrix3x3& m = trans.getBasis();
  // add Euler angles around XYZ to mouse moves
  const btScalar ax = y + btAtan2( -m[1].z(), m[2].z() );
  //btScalar ay = btAsin( m[0].z() );  // not used
  const btScalar az = x + btAtan2( -m[0].y(), m[0].x() );
  // force Y angle to 0
  const btScalar cx = btCos(ax);
  const btScalar sx = btSin(ax);
  const btScalar cz = btCos(az);
  const btScalar sz = btSin(az);
  trans.setBasis(btMatrix3x3(
          cz, -sz, 0,
          cx*sz, cx*cz, -sx,
          sx*sz, cz*sx, cx
          ));
}


/// Internal exception class to abort
class AbortException: public std::exception {};


void Display::run()
{
  if(is_running_) {
    throw(Error("recursive call to run()"));
  }
  if(!physics_) {
    throw(Error("no physics attached to display"));
  }
  // window not created yet, force resize() to create it
  if(!windowInitialized()) {
    resize(screen_x_, screen_y_, fullscreen_);
  }

  unsigned int step_dt = (unsigned int)(1000.0*physics_->getStepDt());
  unsigned long time;
  unsigned long time_last_disp, time_step;
  signed long time_wait;

  try {
    is_running_ = true;
    time_last_disp = time_step = SDL_GetTicks();
    for(;;) {
      time = SDL_GetTicks();
      while(time >= time_step) {
        if(!paused_) {
          physics_->step();
        }
        time_step += (unsigned long)(step_dt * time_scale_);
      }
      if(time - time_last_disp >= 1000.0/fps_) {
        processEvents();
        update();
        time_last_disp = time;
      }

      time_wait = MIN(time_step-time, 1000.0/fps_);
      if(time_wait > 0) {
        SDL_Delay(time_wait);
      }
    }
  } catch(const AbortException&) {
    is_running_ = false;
  } catch(...) {
    is_running_ = false;
    throw;
  }
}

void Display::abort() const
{
  if(!is_running_) {
    throw(Error("cannot abort, not running"));
  }
  throw AbortException();
}


/** @name PNG related declarations
 */
//@{

#define PNG_ERROR_SIZE  256

typedef struct
{
  char msg[PNG_ERROR_SIZE];
} png_error_data;


static void png_handler_error(png_struct* png_ptr, const char* msg)
{
  png_error_data* error = (png_error_data*)png_get_error_ptr(png_ptr);
  strncpy(error->msg, msg, PNG_ERROR_SIZE);
  error->msg[PNG_ERROR_SIZE-1] = '\0';
  longjmp(png_jmpbuf(png_ptr), 1);
}

static void png_handler_warning(png_struct* /*png_ptr*/, const char* msg)
{
  LOG("PNG: warning: %s", msg);
}

//@}

void Display::savePNGScreenshot(const std::string& filename)
{
  if(!screen_) {
    throw(Error("display not opened"));
  }

  try {
    // Open output file
    FILE* fp = fopen(filename.c_str(), "wb");
    if(!fp) {
      throw(Error("cannot open file '%s' for writing", filename.c_str()));
    }
    auto fp_deleter = [](FILE* fp) { fclose(fp); };
    std::unique_ptr<FILE, decltype(fp_deleter)> fp_safe(fp, fp_deleter);

    png_error_data error;

    // PNG initializations

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, &error,
        png_handler_error, png_handler_warning
        );
    if(!png_ptr) {
      throw(Error("png_create_write struct failed"));
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      throw(Error("png_create_info_struct failed"));
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
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

    std::unique_ptr<unsigned char[]> pixels(new unsigned char[4*screen_x_*screen_y_]);
    glReadPixels(0, 0, screen_x_, screen_y_, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
    std::unique_ptr<png_bytep[]> row_pointers(new png_bytep[screen_y_]);
    for(int i=0; i<screen_y_; i++) {
      row_pointers[screen_y_-i-1] = (png_bytep)&pixels[3*i*screen_x_];
    }
    png_set_rows(png_ptr, info_ptr, row_pointers.get());

    // Write data
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

  } catch(const Error& e) {
    throw(Error("SDL: cannot save screenshot: %s", e.what()));
  }
}


void Display::drawString(const std::string& s, int x, int y, Color4 color, void* font)
{
  y = screen_y_ - y;
  glColor4fv(color);
  glRasterPos2f(x,y);
  for(unsigned int i=0; i<s.size(); i++) {
    glutBitmapCharacter(font, s[i]);
  }
}


void Display::windowInit()
{
  if(windowInitialized()) {
    throw(Error("window already initialized")); // should not happen
  }

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
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

  if(antialias > 0) {
    if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) < 0) {
      throw(Error("SDL: cannot enable multisample buffers"));
    }
    if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialias) < 0) {
      throw(Error("SDL: cannot set multisample sample count to %d", antialias));
    }
  }
}

void Display::windowDestroy()
{
  SDL_Quit();
}


void Display::sceneInit()
{
  // Clear the background color
  glClearColor(bg_color_[0], bg_color_[1],
      bg_color_[2], bg_color_[3]);
  glViewport(0, 0, screen_x_, screen_y_);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  const GLfloat light_pos[4] = {0, 0.3_mf, 0.8_mf, 0};
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


bool Display::callOrCreateDisplayList(const void* key)
{
  DisplayListContainer::const_iterator it = display_lists_.find(key);
  if(it != display_lists_.end()) {
    glCallList( (*it).second );
    return false;
  }

  GLuint id = glGenLists(1);
  if(id == 0) {
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

  while(SDL_PollEvent(&event)) {
    it_h = handlers_.find(event);
    if(it_h != handlers_.end()) {
      (*it_h).second(this, &event);
    }
  }
}

void Display::setHandler(const SDL_Event& ev, Display::EventCallback cb)
{
  if(!cb) {
    handlers_.erase(ev);
  } else {
    handlers_[ev] = cb;
  }
}

void Display::setDefaultHandlers()
{
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
#ifdef _WIN32
  event.key.keysym.sym = SDLK_w;
#else
  event.key.keysym.sym = SDLK_z;
#endif
  setHandler(event, handlerCamAhead);
  event.key.keysym.sym = SDLK_s;
  setHandler(event, handlerCamBack);
#ifdef _WIN32
  event.key.keysym.sym = SDLK_a;
#else
  event.key.keysym.sym = SDLK_q;
#endif
  setHandler(event, handlerCamLeft);
  event.key.keysym.sym = SDLK_d;
  setHandler(event, handlerCamRight);
#ifdef _WIN32
  event.key.keysym.sym = SDLK_q;
#else
  event.key.keysym.sym = SDLK_a;
#endif
  setHandler(event, handlerCamUp);
  event.key.keysym.sym = SDLK_e;
  setHandler(event, handlerCamDown);
  event.key.keysym.sym = SDLK_r;
  setHandler(event, handlerCamReset);
}


void Display::handlerQuit(Display* d, const SDL_Event*)
{
  d->abort();
}

void Display::handlerResize(Display* d, const SDL_Event* event)
{
  d->resize(event->resize.w, event->resize.h);
}

void Display::handlerPause(Display* d, const SDL_Event*)
{
  d->paused_ = !d->paused_;
}

void Display::handlerCamMouse(Display* d, const SDL_Event* event)
{
  d->camera_.mouseMove(
      event->motion.xrel*d->camera_mouse_coef_,
      event->motion.yrel*d->camera_mouse_coef_
      );
}

void Display::handlerCamAhead(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() -= d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(2);
}

void Display::handlerCamBack(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() += d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(2);
}

void Display::handlerCamLeft(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() -= d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(0);
}

void Display::handlerCamRight(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() += d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(0);
}

void Display::handlerCamUp(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() -= d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(1);
}

void Display::handlerCamDown(Display* d, const SDL_Event*)
{
  d->camera_.trans.getOrigin() += d->camera_step_linear_ * d->camera_.trans.getBasis().getRow(1);
}

void Display::handlerCamReset(Display* d, const SDL_Event*)
{
  d->camera_.trans = btTransform(
      btQuaternion(0, -M_PI/6, 0),
      btVector3(0.0_m, -2.0_m, 3.0_m)
      );
}


bool Display::EventCmp::operator()(const SDL_Event& a, const SDL_Event& b)
{
  if(a.type == b.type) {
    switch(a.type) {
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


