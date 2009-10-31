#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include <map>
#include <set>
#include "global.h"


///@file

class EventHandler;
class OSDMessage;


/** @brief Display and interface events
 *
 * Display is not needed for the simulation to run.
 */
class Display: public SmartObject
{
  friend class LuaDisplay;
public:
  Display();
  virtual ~Display();

  /// Init video display (using configuration values)
  void init();
  bool isInitialized() { return SDL_WasInit(SDL_INIT_VIDEO) != 0; }

  /** @brief Resize the screen and/or toggle fullscreen
   * @param width   window width
   * @param height  window height
   * @param mode    window if 0, fullscreen if >0, current state if <0
   */
  void resize(int width, int height, int mode=-1);

  /// Update display
  void update();

  /** @name Screenshots
   *
   * Screenshots are scheduled to be taken after next drawn frame.
   * Only the last filename will be taken in account.
   */
  //@{

private:
  /** @brief Schedule a screenshot.
   * The filename string is copied and can safely be immediately freed by the
   * caller.
   */
  void savePNGScreenshot(const char *filename);
  /// Save a PNG screenshot into a file
  void doSavePNGScreenshot(const char *filename);
  /// Filename of the scheduled screenshot, or \e NULL
  char *screenshot_filename;

  //@}

private:
  SDL_Surface *screen;

  int screen_x; ///< Screen width
  int screen_y; ///< Screen height
  bool fullscreen;

  void windowInit();
  void windowDestroy();
  void sceneInit();
  void sceneDestroy();


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
  typedef struct
  {
    btVector3  cart;
    btSpheric3 spheric;
    SmartPtr<Object> obj;
  } CameraPoint;

  const CameraPoint &getCameraEye()    const { return camera_eye;    }
  const CameraPoint &getCameraTarget() const { return camera_target; }
  int getCameraMode() const { return camera_mode;   }

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
  CameraPoint camera_eye;
  CameraPoint camera_target;
  int camera_mode;

  //@}


  /** @name Events
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
  //@{
public:

  /// Process SDL events
  void processEvents();

  /** @brief Event comparison function class
   *
   * Events are first ordered by type, then by specific fields:
   *  - keydown/up: <tt>keysym.sym</tt>
   *  - mouse motion: <tt>state</tt>
   *  - mouse button: <tt>button</tt>
   *  - user: <tt>code</tt>
   *  - others: none
   */
  class EventCmp
  {
  public:
    bool operator()(const SDL_Event &a, const SDL_Event &b);
  };

  /** @brief Event handler
   */
  class EventHandler
  {
  public:
    typedef void (*Callback)(Display *, const SDL_Event *);

    /// C++ constructor
    EventHandler(Callback cb): ptr_cb(cb), L(NULL), ref_cb(LUA_NOREF) {}
    /// Lua constructor
    EventHandler(lua_State *L, int ref);
    /// Copy constructor, to copy the Lua reference
    EventHandler(const EventHandler &h);

    virtual ~EventHandler();

    void operator()(Display *d, const SDL_Event *ev) const;

  private:
    /// C++ callback
    Callback ptr_cb;
    /// Lua callback state
    lua_State *L;
    /// Lua callback reference
    int ref_cb;
  };

  /// Add, replace or remove an event handler
  void setHandler(const SDL_Event &ev, const EventHandler &h);

private:
  typedef std::map<SDL_Event, EventHandler, EventCmp> EventHandlerContainer;
  /// Event handlers
  EventHandlerContainer handlers;

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

  //@}


  /** @name On Screen Display
   */
  //@{

public:

  /** @brief Draw a text string using given font
   * @note y=0 is the top of the screen
   */
  void drawString(const char *s, int x, int y, Color4 color, void *font);
  std::set< SmartPtr<OSDMessage> > &getOsds() { return osds; }

private:
  /// Displayed OSDs
  std::set< SmartPtr<OSDMessage> > osds;

  //@}

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
  virtual const char *getText() = 0;
  virtual int getX() = 0;
  virtual int getY() = 0;
  virtual Color4 getColor() = 0;
  //@}
};

/** @brief Simple OSD
 *
 * Simple OSD implementation with public access to OSD properties.
 * This is the base OSD class used in Lua bindings.
 */
class OSDSimple: public OSDMessage
{
public:
  OSDSimple(): text(NULL), x(0),y(0), color(Color4::black()) {}
  virtual ~OSDSimple() {}

  virtual const char *getText() { return text; }
  virtual int getX() { return x; }
  virtual int getY() { return y; }
  virtual Color4 getColor() { return color; }

public:
  const char *text;
  int x, y;
  Color4 color;
};

/** @brief OSD for Lua bindings
 *
 * Attributes associated to OSD properties may be static values or functions.
 */
class OSDLua: public OSDMessage
{
public:
  OSDLua(lua_State *L, int ref_obj);
  virtual ~OSDLua();

  /** @name Common accessors
   */
  //@{
  virtual const char *getText();
  virtual int getX();
  virtual int getY();
  virtual Color4 getColor();
  //@}

protected:
  /// Lua instance state
  lua_State *L;
  /// Lua instance reference
  int ref_obj;
  /// Lua text reference
  int ref_text;
};


#endif
