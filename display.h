#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include "object.h"

///@file


class Display
{
public:
  Display();
  ~Display();

  /// Resize the screen
  void resize(int width, int height);

  /// Update display
  void update();

  /** @brief Process SDL events
   * @todo It's not display... Rename? Put it in another module?
   */
  void handle_events();

  /** @brief Camera modes
   *
   * Two modes ares used:
   *  - fixed: camera moves around a fixed point;
   *  - free: free moves.
   */
  typedef enum { CAM_FIXED, CAM_FREE } CameraMode;

  void set_camera_mode(CameraMode mode);


private:
  SDL_Surface *screen;

  Uint8  sdl_bpp;   ///< bits per pixel
  Uint32 sdl_flags; ///< SDL video mode flags

  int screen_x; ///< Screen width
  int screen_y; ///< Screen height

  /** @brief Camera position and direction
   *
   * Depending on the mode, eye and target are given in cartesian or spherical
   * coordinates.
   */
  struct
  {
    float eye[3];
    float target[3];
    CameraMode mode;
  } camera;

  void window_init();
  void window_destroy();
  void scene_init();
  void scene_destroy();

};


#endif
