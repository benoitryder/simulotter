#ifndef DISPLAY_H_
#define DISPLAY_H_

///@file

#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <string>
#include <map>
#include <set>
#include "smart.h"
#include "colors.h"

class Physics;
class Object;
class Display;
class OSDMessage;


/** @brief Event handler interface.
 *
 * Handlers are associated to SDL events. Each input event is checked given
 * handlers.
 *
 * Event type are compared using type and type specific fields (e.g. \e
 * keysym for keyboard events), some fields are not compared (e.g. mouse
 * coordinates for motion events).
 *
 * Their could not have several active handlers matching a same event.
 *
 * @note Key repeat is enabled.
 */
class DisplayEventHandler: public SmartObject
{
public:
  /** @brief Event comparison function class
   *
   * Events are first ordered by type, then by specific fields:
   *  - keydown/up: <tt>keysym.sym</tt>
   *  - mouse motion: <tt>state</tt>
   *  - mouse button: <tt>button</tt>
   *  - user: <tt>code</tt>
   *  - others: none
   */
  class Cmp
  {
  public:
    bool operator()(const SDL_Event &a, const SDL_Event &b);
  };

  virtual ~DisplayEventHandler() {}

  typedef void (*Callback)(Display *, const SDL_Event *);
  virtual void process(Display *d, const SDL_Event *ev) const = 0;
};


/** @brief Display and interface events
 *
 * Display is not needed for the simulation to run.
 */
class Display: public SmartObject
{
public:
  /** @name Configuration values.
   *
   * @note Changing some values may not have any effect on already drawn
   * objects due to usage of display lists.
   */
  //@{

  /// Gap between contiguous surfaces.
  static btScalar draw_epsilon;
  /// Slices and stacks for GLUT geometry objects.
  static unsigned int draw_div;
  /// Multisampling count (0 to disable).
  static unsigned int antialias;

  //@}

  Display();
  virtual ~Display();

  SmartPtr<Physics> physics_; ///< Currently drawn Physics
public:
  Physics *getPhysics() { return physics_; }
  void setPhysics(Physics *ph) { physics_ = ph; }

  /** @name Dynamic configuration values.
   *
   * These values can be modified while the simulation is running.
   *
   * @todo Checks should be added for some values.
   */
  //@{

  /** @brief Time scale coefficient.
   *
   * If greater than 1, slow down the simulation.
   * If less than 1, speed up the simulation.
   */
  float time_scale;
  /** @brief Display rate.
   * @note It also defines the event handling rate.
   */
  float fps;

  Color4 bg_color;

  /// Step for camera angle moves
  float camera_step_angle;
  /// Step for camera linear moves
  float camera_step_linear;
  /// Coefficient for mouse camera moves
  float camera_mouse_coef;

  /// Perspective field of view (in degrees)
  float perspective_fov;
  /// Near clipping plance distance
  float perspective_near;
  /// Far clipping plance distance
  float perspective_far;

  //@}


  /** @brief Resize the screen and/or toggle fullscreen
   * @param width   window width
   * @param height  window height
   * @param mode    window if 0, fullscreen if >0, current state if <0
   */
  void resize(int width, int height, int mode=-1);

  /// Update display
  void update();

  /** @brief Run simulation display.
   *
   * Initialize the display (if needed) and display simulation.
   * Timings are given by fps and time_scale fields.
   *
   * @note This method never returns.
   */
  void run();

  /// Save a PNG screenshot into a file
  void savePNGScreenshot(const std::string &filename);

private:
  SDL_Surface *screen_;

  int screen_x_; ///< Screen width
  int screen_y_; ///< Screen height
  bool fullscreen_;


  
  bool windowInitialized() const { return SDL_WasInit(SDL_INIT_VIDEO) != 0; }
  void windowInit();
  void windowDestroy();
  void sceneInit();
  void sceneDestroy();


  /** @name Display lists.
   *
   * Display lists are stored in an associative map whose keys are typically
   * pointers to elements drawn by the display list.
   *
   * @todo Allow an object to destroy its display list.
   */
  //@{
public:

  /** @brief Call an existing playlist or starts a new one.
   *
   * If a list for the given key exists, it is called and false is returned.
   * Otherwise, a new list is created, ready to be compiled, and true is
   * returned. endDisplayList() must be called when the list is completed.
   *
   * @return true if a new list has been created, false otherwise.
   */
  bool callOrCreateDisplayList(void *key);
  /// End currently initialized display list.
  void endDisplayList();

private:
  typedef std::map<void *, GLuint> DisplayListContainer;
  DisplayListContainer display_lists_;

  //@}


  /** @name Camera
   *
   * There are three modes for eye and target; depending on it eye and target
   * values are cartesian or spherical coordinates:
   *  - fixed: cartesian coordinates, does not move
   *  - ref: spherical offset from the other point (eye or target)
   *  - object: object position with spherical offset
   */
  //@{
public:

  /// Camera modes
  enum CamMode
  {
    CAM_EYE_FIXED  = 0x01,
    CAM_EYE_REL    = 0x02,
    CAM_EYE_OBJECT = 0x04,
    CAM_EYE_MASK   = 0xff,

    CAM_TARGET_FIXED   = 0x0100,
    CAM_TARGET_REL     = 0x0200,
    CAM_TARGET_OBJECT  = 0x0400,
    CAM_TARGET_MASK    = 0xff00,

    // Aliases
    CAM_FREE    = CAM_EYE_FIXED  | CAM_TARGET_REL,
    CAM_FIXED   = CAM_EYE_REL    | CAM_TARGET_FIXED,
    CAM_FOLLOW  = CAM_EYE_REL    | CAM_TARGET_OBJECT,
    CAM_ONBOARD = CAM_EYE_OBJECT | CAM_TARGET_REL,
    CAM_LOOK    = CAM_EYE_FIXED  | CAM_TARGET_OBJECT,
  };

  /** @brief Camera point position
   *
   * @note \e cart and \e spheric are not used simultaneously.
   */
  struct CameraPoint
  {
    btVector3  cart;
    btSpheric3 spheric;
    SmartPtr<Object> obj;
  };

  const CameraPoint &getCameraEye()    const { return camera_eye_;    }
  const CameraPoint &getCameraTarget() const { return camera_target_; }
  int getCameraMode() const { return camera_mode_; }

  /** @brief Compute eye and target cartesian global positions
   *
   * getCameraEye() and getCameraTarget() return values which may be relative
   * to each other, this method returns global positions.
   */
  void getCameraPos(btVector3 &eye_pos, btVector3 &target_pos) const;

  /** @brief Change camera mode
   * @note Object(s) must be set before choosing an object mode.
   */
  void setCameraMode(int mode);

private:
  CameraPoint camera_eye_;
  CameraPoint camera_target_;
  int camera_mode_;

  //@}


public:

  /// Process SDL events
  void processEvents();

  /// Add, replace or remove an event handler
  void setHandler(const SDL_Event &ev, DisplayEventHandler *h);

private:
  typedef std::map<SDL_Event, SmartPtr<DisplayEventHandler>,
          DisplayEventHandler::Cmp> EventHandlerContainer;
  /// Event handlers
  EventHandlerContainer handlers_;

  /** @name Default handlers
   */
  //@{
  static void handlerQuit    (Display *d, const SDL_Event *event) {throw(0);}
  static void handlerResize  (Display *d, const SDL_Event *event);
  static void handlerPause   (Display *d, const SDL_Event *event);
  static void handlerCamMouse(Display *d, const SDL_Event *event);
  static void handlerCamAhead(Display *d, const SDL_Event *event);
  static void handlerCamBack (Display *d, const SDL_Event *event);
  static void handlerCamLeft (Display *d, const SDL_Event *event);
  static void handlerCamRight(Display *d, const SDL_Event *event);
  static void handlerCamUp   (Display *d, const SDL_Event *event);
  static void handlerCamDown (Display *d, const SDL_Event *event);
  //@}


  /** @name On Screen Display
   */
  //@{

public:

  /** @brief Draw a text string using given font
   * @note y=0 is the top of the screen
   */
  void drawString(const std::string &s, int x, int y, Color4 color, void *font);
  std::set< SmartPtr<OSDMessage> > &getOsds() { return osds_; }

private:
  /// Displayed OSDs
  std::set< SmartPtr<OSDMessage> > osds_;

  //@}
};


/** @brief Basic event handler.
 */
class BasicEventHandler: public DisplayEventHandler
{
 public:
  BasicEventHandler(Callback cb): cb_(cb) {}
  virtual void process(Display *d, const SDL_Event *ev) const { cb_(d,ev); }
 private:
  Callback cb_;  /// C++ callback
};


/** @brief OSD messages interface
 *
 * Base class for text messages displayed on screen.
 */
class OSDMessage: public SmartObject
{
public:
  OSDMessage() {}
  virtual ~OSDMessage() {}

  /** @name Common accessors
   */
  //@{
  virtual std::string getText() = 0;
  virtual int getX() = 0;
  virtual int getY() = 0;
  virtual Color4 getColor() = 0;
  //@}
};

/** @brief Simple OSD
 *
 * Simple OSD implementation with public access to OSD properties.
 */
class OSDSimple: public OSDMessage
{
public:
  OSDSimple(): text_(NULL), x_(0),y_(0), color_(Color4::black()) {}
  virtual ~OSDSimple() {}

  virtual std::string getText() { return text_; }
  virtual int getX() { return x_; }
  virtual int getY() { return y_; }
  virtual Color4 getColor() { return color_; }

private:
  std::string text_;
  int x_, y_;
  Color4 color_;
};


#endif
