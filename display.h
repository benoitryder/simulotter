#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include <map>
#include "object.h"

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
    window_init();
  }

  /// Resize the screen
  void resize(int width, int height);

  /// Update display
  void update();

private:
  SDL_Surface *screen;

  Uint8  sdl_bpp;   ///< bits per pixel
  Uint32 sdl_flags; ///< SDL video mode flags

  int screen_x; ///< Screen width
  int screen_y; ///< Screen height

  void window_init();
  void window_destroy();
  void scene_init();
  void scene_destroy();


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

  int get_camera_mode() const { return camera.mode;   }

  const float *get_camera_eye()    const { return camera.eye;    }
  const float *get_camera_target() const { return camera.target; }

  /** @brief Compute eye and target cartesian positions
   * get_camera_eye() and get_camera_target() return values, this method
   * returns global positions.
   */
  void get_camera_pos(float eye_pos[3], float target_pos[3]);

  /** @brief Change camera mode
   * @note Object(s) must be set before choosing an object mode.
   */
  void set_camera_mode(int mode);
  void set_camera_eye    (float v0, float v1, float v2) { camera.eye[0]    = v0; camera.eye[1]    = v1; camera.eye[2]    = v2; }
  void set_camera_target (float v0, float v1, float v2) { camera.target[0] = v0; camera.target[1] = v1; camera.target[2] = v2; }
  void move_camera_eye   (float v0, float v1, float v2) { camera.eye[0]   += v0; camera.eye[1]   += v1; camera.eye[2]   += v2; }
  void move_camera_target(float v0, float v1, float v2) { camera.target[0]+= v0; camera.target[1]+= v1; camera.target[2]+= v2; }
  void set_camera_eye_obj(Object *o) { camera.eye_obj = o; }
  void set_camera_target_obj(Object *o) { camera.target_obj = o; }

private:
  /** @brief Camera position and direction
   */
  struct
  {
    float eye[3];
    float target[3];
    Object *eye_obj;
    Object *target_obj;
    int mode;
  } camera;

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
  void handle_events();

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
  void set_handler(const SDL_Event &event, EventHandler handler) { handlers[event] = handler; }

private:
  /// Event handlers
  std::map<SDL_Event, EventHandler, EventCmp> handlers;

  /** @name Default handlers
   */
  //@{
  static void handler_quit     (Display *d, const SDL_Event &event) {throw(0);}
  static void handler_resize   (Display *d, const SDL_Event &event);
  static void handler_pause    (Display *d, const SDL_Event &event);
  static void handler_cam_mode (Display *d, const SDL_Event &event);
  static void handler_cam_mouse(Display *d, const SDL_Event &event);
  static void handler_cam_ahead(Display *d, const SDL_Event &event);
  static void handler_cam_back (Display *d, const SDL_Event &event);
  static void handler_cam_left (Display *d, const SDL_Event &event);
  static void handler_cam_right(Display *d, const SDL_Event &event);
  static void handler_cam_up   (Display *d, const SDL_Event &event);
  static void handler_cam_down (Display *d, const SDL_Event &event);
  //@}

  //@}

};


#endif
