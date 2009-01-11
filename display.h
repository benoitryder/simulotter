#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include <map>
#include "global.h"


///@file


/** @brief Display and interface events
 */
class Display
{
public:
  Display();
  ~Display();

  /// Init and start video display
  void init()
  {
    windowInit();
  }

  /// Resize the screen
  void resize(int width, int height);

  /// Update display
  void update();

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
  enum
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
    Object *obj;
  } CameraPoint;

  const CameraPoint &getCameraEye()    const { return camera_eye;    }
  const CameraPoint &getCameraTarget() const { return camera_target; }
  int getCameraMode() const { return camera_mode;   }

  /** @brief Compute eye and target cartesian global positions
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
   * Handlers are associated to SDL events. Each input event is checked agains
   * keys.
   * Event type are compared, then additional type specific comparisons are
   * made (e.g. \e keysym for keyboard events).
   * Some fields are not compared (e.g. mouse coordinates for motion events).
   *
   * @note Key repeat is enabled.
   */
  //@{
public:

  /// Process SDL events
  void handleEvents();

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

  /// Event handler type
  typedef void (*EventHandler)(Display *d, const SDL_Event &event);

  /// Add or replace an event handler
  void setHandler(const SDL_Event &event, EventHandler handler) { handlers[event] = handler; }

private:
  /// Event handlers
  std::map<SDL_Event, EventHandler, EventCmp> handlers;

  /** @name Default handlers
   */
  //@{
  static void handlerQuit    (Display *d, const SDL_Event &event) {throw(0);}
  static void handlerResize  (Display *d, const SDL_Event &event);
  static void handlerPause   (Display *d, const SDL_Event &event);
  static void handlerCamMode (Display *d, const SDL_Event &event);
  static void handlerCamMouse(Display *d, const SDL_Event &event);
  static void handlerCamAhead(Display *d, const SDL_Event &event);
  static void handlerCamBack (Display *d, const SDL_Event &event);
  static void handlerCamLeft (Display *d, const SDL_Event &event);
  static void handlerCamRight(Display *d, const SDL_Event &event);
  static void handlerCamUp   (Display *d, const SDL_Event &event);
  static void handlerCamDown (Display *d, const SDL_Event &event);
  //@}

  //@}

};


#endif
