#ifndef DISPLAY_H_
#define DISPLAY_H_

///@file

#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include <string>
#include <map>
#include <set>
#include <functional>
#include "smart.h"
#include "physics.h"
#include "colors.h"

class Physics;
class Object;
class Display;
class OSDMessage;


/** @brief Display and interface events
 *
 * Display is not needed for the simulation to run.
 */
class Display: public SmartObject
{
 public:
  /** @name Configuration values
   *
   * @note Changing some values may not have any effect on already drawn
   * objects due to usage of display lists.
   */
  //@{

  /// Gap between contiguous surfaces
  static btScalar draw_epsilon;
  /// Slices and stacks for GLUT geometry objects
  static unsigned int draw_div;
  /// Multisampling count (0 to disable)
  static unsigned int antialias;

  //@}

  Display();
  virtual ~Display();

  SmartPtr<Physics> physics_; ///< Currently drawn Physics
 public:
  Physics* getPhysics() const { return physics_; }
  void setPhysics(Physics* ph) { physics_ = ph; }

  /** @name Dynamic configuration values
   *
   * These values can be modified while the simulation is running.
   *
   * @todo Checks should be added for some values.
   */
  //@{

  /** @brief Time scale coefficient
   *
   * If greater than 1, slow down the simulation.
   * If less than 1, speed up the simulation.
   */
  float time_scale_;
  /** @brief Display rate
   * @note It also defines the event handling rate.
   */
  float fps_;
  /// If true, simulation is not stepped in run()
  bool paused_;

  Color4 bg_color_;

  /// Step for camera linear moves
  float camera_step_linear_;
  /// Coefficient for mouse camera moves
  float camera_mouse_coef_;

  //@}


  /** @brief Resize the screen and/or toggle fullscreen
   * @param width   window width
   * @param height  window height
   * @param mode    window if 0, fullscreen if >0, current state if <0
   */
  void resize(int width, int height, int mode=-1);

  int getScreenWidth() const { return screen_x_; }
  int getScreenHeight() const { return screen_y_; }

  /// Update display
  void update();

  /// Close display window
  void close();

  /** @brief Run simulation display
   *
   * Initialize the display (if needed) and display simulation.
   * Timings are given by fps and time_scale_ fields.
   */
  void run();

  /// Abort a current call to run()
  void abort() const;


  /// Save a PNG screenshot into a file
  void savePNGScreenshot(const std::string& filename);

 private:
  SDL_Surface* screen_;

  int screen_x_; ///< Screen width
  int screen_y_; ///< Screen height
  bool fullscreen_;

  bool is_running_;  ///< True if run() is being called

  bool windowInitialized() const { return SDL_WasInit(SDL_INIT_VIDEO) != 0; }
  void windowInit();
  void windowDestroy();
  void sceneInit();
  void sceneDestroy();


  /** @name Display lists
   *
   * Display lists are stored in an associative map whose keys are typically
   * pointers to elements drawn by the display list.
   *
   * @todo Allow an object to destroy its display list.
   */
  //@{
 public:

  /** @brief Call an existing playlist or starts a new one
   *
   * If a list for the given key exists, it is called and false is returned.
   * Otherwise, a new list is created, ready to be compiled, and true is
   * returned. endDisplayList() must be called when the list is completed.
   *
   * @return true if a new list has been created, false otherwise.
   */
  bool callOrCreateDisplayList(const void* key);
  /// End currently initialized display list
  void endDisplayList();

 private:
  typedef std::map<const void*, GLuint> DisplayListContainer;
  DisplayListContainer display_lists_;

  //@}

 public:

  /** @brief Camera
   *
   * \e trans gives the camera transformation from the camera referential.
   * If \e obj is not \e NULL the camera referential is the object
   * transformation. Otherwise, the world's referential is used.
   *
   * The camera is oriented along the negative Z axis (towards the ground) with
   * vertical Y axis and horizontal X axis.
   */
  struct Camera
  {
    /// Camera position and orientation
    btTransform trans;
    /// Reference object for embedded camera
    SmartPtr<Object> obj;

    /// Vertical field of view (in degrees)
    float fov;
    /// Near clipping plane distance
    float z_near;
    /// Far clipping plane distance
    float z_far;

    /// Default constructor, set default values
    Camera();
    /** @brief Move the camera as expected for mouse moves
     *
     * \e x and \y are expressed in an arbitrary unit.
     */
    void mouseMove(btScalar x, btScalar y);
  };

  /// Main display camera
  Camera camera_;

  //@}

  /** @name Event handling
   *
   * Handlers are associated to SDL events. Each input event is checked against
   * given handlers.
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
  /// Event handler callback
  typedef std::function<void (Display*, const SDL_Event*)> EventCallback;
  /// Process SDL events
  void processEvents();
  /// Add, replace or remove an event handler
  void setHandler(const SDL_Event& ev, EventCallback cb);
  /// Set default handlers
  void setDefaultHandlers();

 private:
  /** @brief Event comparison function class
   *
   * Events are first ordered by type, then by specific fields:
   *  - keydown/up: <tt>keysym.sym</tt>
   *  - mouse motion: <tt>state</tt>
   *  - mouse button: <tt>button</tt>
   *  - user: <tt>code</tt>
   *  - others: none
   */
  struct EventCmp {
    bool operator()(const SDL_Event& a, const SDL_Event& b);
  };

  typedef std::map<SDL_Event, EventCallback, EventCmp> EventHandlerContainer;
  /// Event handlers
  EventHandlerContainer handlers_;

  //@}

  /** @name Default handler methods
   */
  //@{
  static void handlerQuit(Display* d, const SDL_Event* event);
  static void handlerResize(Display* d, const SDL_Event* event);
  static void handlerPause(Display* d, const SDL_Event* event);
  static void handlerCamMouse(Display* d, const SDL_Event* event);
  static void handlerCamAhead(Display* d, const SDL_Event* event);
  static void handlerCamBack(Display* d, const SDL_Event* event);
  static void handlerCamLeft(Display* d, const SDL_Event* event);
  static void handlerCamRight(Display* d, const SDL_Event* event);
  static void handlerCamUp(Display* d, const SDL_Event* event);
  static void handlerCamDown(Display* d, const SDL_Event* event);
  static void handlerCamReset(Display* d, const SDL_Event* event);
  //@}


  /** @name On Screen Display
   */
  //@{

 public:

  /** @brief Draw a text string using given font
   * @note y=0 is the top of the screen
   */
  void drawString(const std::string& s, int x, int y, Color4 color, void* font);
  std::set<SmartPtr<OSDMessage>>& getOsds() { return osds_; }

 private:
  /// Displayed OSDs
  std::set<SmartPtr<OSDMessage>> osds_;

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
  virtual std::string getText() = 0;
  virtual int getX() = 0;
  virtual int getY() = 0;
  virtual Color4 getColor() = 0;
  //@}
};


#endif
