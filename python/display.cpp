#include "python/common.h"
#include <boost/python/raw_function.hpp>
#include "display.h"
#include "physics.h"
#include "object.h"


static void Display_resize(Display& d, int width, int height, py::object mode)
{
  int mode_cpp;
  if(mode.ptr() == py::object().ptr()) {
    mode_cpp = -1; // none: use current state
  } else {
    mode_cpp = mode ? 1 : 0;
  }
  d.resize(width, height, mode_cpp);
}
static py::object Display_get_screen_size(const Display& d)
{
  return py::make_tuple( d.getScreenWidth(), d.getScreenHeight() );
}


// keep some exported elements, used by methods
static py::object py_event_cls;
static py::object py_key_enum;

static void Display_handler_cb(py::object cb, Display* d, const SDL_Event* event)
{
  py::object py_ev = py_event_cls(); // new instance
  // fill py_ev with event infos
  py_ev.attr("type") = event->type;
  switch(event->type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      py_ev.attr("key") = event->key.keysym.sym;
      py_ev.attr("mod") = event->key.keysym.mod;
      break;
    case SDL_MOUSEMOTION:
      py_ev.attr("state") = event->motion.state;
      py_ev.attr("pos") = py::make_tuple(event->motion.x, event->motion.y);
      py_ev.attr("rel") = py::make_tuple(event->motion.xrel, event->motion.yrel);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      py_ev.attr("button") = event->button.button;
      py_ev.attr("pos") = py::make_tuple(event->button.x, event->button.y);
      break;
    default:
      PyErr_SetString(PyExc_ValueError, "unhandled event type");
      throw py::error_already_set();
  }

  py::call<void>(cb.ptr(), py::ptr(d), py_ev);
}

static void Display_set_handler(Display& d, py::object cb, Uint8 type, py::dict kw)
{
  Display::EventCallback cpp_cb;
  if(cb.ptr() == Py_None) {
    cpp_cb = NULL;
  } else {
    if(!PyCallable_Check(cb.ptr())) {
      PyErr_SetString(PyExc_TypeError, "callback is not callable");
      throw py::error_already_set();
    }
    cpp_cb = boost::bind(Display_handler_cb, cb, _1, _2);
  }

  SDL_Event ev;
  ev.type = type;
  switch(ev.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      py::object okey = kw["key"];
      py::extract<SDLKey> xkey(okey);
      if(xkey.check()) {
        ev.key.keysym.sym = xkey;
      } else if(PyString_Check(okey.ptr()) && PyString_GET_SIZE(okey.ptr()) == 1) {
        // single character: use ord()
        // (code from Python implementation)
        ev.key.keysym.sym = static_cast<SDLKey>(*PyString_AS_STRING(okey.ptr()));
      } else {
        // try enum name
        ev.key.keysym.sym = py::extract<SDLKey>(py_key_enum.attr(okey));
      }
    } break;
    case SDL_MOUSEMOTION:
      ev.motion.state = py::extract<Uint8>(kw["state"]);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      ev.button.button = py::extract<Uint8>(kw["button"]);
      break;
    default:
      PyErr_SetString(PyExc_ValueError, "invalid event type");
      throw py::error_already_set();
  }

  d.setHandler(ev, cpp_cb);
}

// simple wrapper to expand arguments
static py::tuple Display_set_handler_wrap(py::tuple args, py::dict kw)
{
  if(py::len(args) != 3) {
    PyErr_SetString(PyExc_TypeError, "expected 3 arguments");
    throw py::error_already_set();
  }
  Display_set_handler(
      py::extract<Display&>(args[0]), py::object(args[1]),
      py::extract<Uint8>(args[2]), kw);
  return py::make_tuple();
}


static btScalar Display_get_draw_epsilon() { return btUnscale(Display::draw_epsilon); }
static void Display_set_draw_epsilon(btScalar v) { Display::draw_epsilon = btScale(v); }

static btTransform Camera_get_trans(const Display::Camera& o) { return btUnscale(o.trans); }
static void Camera_set_trans(Display::Camera& o, const btTransform& tr) { o.trans = btScale(tr); }
static SmartPtr<Object> Camera_get_obj(const Display::Camera& o) { return o.obj; }
static void Camera_set_obj(Display::Camera& o, const SmartPtr<Object>& obj) { o.obj = obj; }
static float Camera_get_z_near(const Display::Camera& o) { return btUnscale(o.z_near); }
static void Camera_set_z_near(Display::Camera& o, float v) { o.z_near = btScale(v); }
static float Camera_get_z_far(const Display::Camera& o) { return btUnscale(o.z_far); }
static void Camera_set_z_far(Display::Camera& o, float v) { o.z_far = btScale(v); }


void python_export_display()
{
  py::scope in_Display = py::class_<Display, SmartPtr<Display>, boost::noncopyable>("Display")
      .def("run", &Display::run)
      .def("abort", &Display::abort)
      .add_property("physics",
                    py::make_function(&Display::getPhysics, py::return_internal_reference<>()),
                    &Display::setPhysics)
      .def("update", &Display::update)
      .def("resize", &Display_resize,
           ( py::arg("width"), py::arg("height"), py::arg("mode")=py::object() ))
      .add_property("screen_size", &Display_get_screen_size)
      .def("close", &Display::close)
      .def("screenshot", &Display::savePNGScreenshot)
      .def("set_handler", py::raw_function(&Display_set_handler_wrap, 3))
      .def("set_default_handlers", &Display::setDefaultHandlers)
      // dynamic configuration
      .def_readwrite("time_scale", &Display::time_scale_)
      .def_readwrite("fps", &Display::fps_)
      .def_readwrite("paused", &Display::paused_)
      .def_readwrite("bg_color", &Display::bg_color_)
      .add_property("camera", py::make_getter(&Display::camera_, py::return_internal_reference<>()))
      // statics
      .add_static_property("draw_epsilon", &Display_get_draw_epsilon, &Display_set_draw_epsilon)
      .def_readwrite("draw_div", &Display::draw_div)
      .def_readwrite("antialias", &Display::antialias)
      ;

  py::class_<Display::Camera, Display::Camera*, boost::noncopyable>("Camera", py::no_init)
      //XXX Camera could be copyable and instantiable
      .add_property("trans", &Camera_get_trans, &Camera_set_trans)
      // property required for None conversions
      .add_property("obj", &Camera_get_obj, &Camera_set_obj)
      .def_readwrite("fov", &Display::Camera::fov)
      .add_property("z_near", &Camera_get_z_near, &Camera_set_z_near)
      .add_property("z_far", &Camera_get_z_far, &Camera_set_z_far)
      .def("mouse_move", &Display::Camera::mouseMove)
      ;

  py_event_cls = py::class_<SDL_Event, boost::noncopyable>("Event");

  // enums

  py::enum_<SDL_EventType>("EventType")
    // export only used/allowed values
    .value("KEYDOWN", SDL_KEYDOWN)
    .value("KEYUP", SDL_KEYUP)
    .value("MOUSEMOTION", SDL_MOUSEMOTION)
    .value("MOUSEBUTTONDOWN", SDL_MOUSEBUTTONDOWN)
    .value("MOUSEBUTTONUP", SDL_MOUSEBUTTONUP)
    .export_values()  // export in Display
    ;

  py::object py_enum_mod = py::enum_<SDLMod>("KMode")
    .value("NONE", KMOD_NONE)
    .value("LSHIFT", KMOD_LSHIFT)
    .value("RSHIFT", KMOD_RSHIFT)
    .value("LCTRL", KMOD_LCTRL)
    .value("RCTRL", KMOD_RCTRL)
    .value("LALT", KMOD_LALT)
    .value("RALT", KMOD_RALT)
    .value("LMETA", KMOD_LMETA)
    .value("RMETA", KMOD_RMETA)
    .value("NUM", KMOD_NUM)
    .value("CAPS", KMOD_CAPS)
    .value("MODE", KMOD_MODE)
    ;
  py_enum_mod.attr("CTRL") = KMOD_CTRL;
  py_enum_mod.attr("SHIFT") = KMOD_SHIFT;
  py_enum_mod.attr("ALT") = KMOD_ALT;
  py_enum_mod.attr("META") = KMOD_META;

  py_key_enum = py::enum_<SDLKey>("Key")
    .value("BACKSPACE", SDLK_BACKSPACE)
    .value("TAB", SDLK_TAB)
    .value("CLEAR", SDLK_CLEAR)
    .value("RETURN", SDLK_RETURN)
    .value("PAUSE", SDLK_PAUSE)
    .value("ESCAPE", SDLK_ESCAPE)
    .value("SPACE", SDLK_SPACE)
    .value("EXCLAIM", SDLK_EXCLAIM)
    .value("QUOTEDBL", SDLK_QUOTEDBL)
    .value("HASH", SDLK_HASH)
    .value("DOLLAR", SDLK_DOLLAR)
    .value("AMPERSAND", SDLK_AMPERSAND)
    .value("QUOTE", SDLK_QUOTE)
    .value("LEFTPAREN", SDLK_LEFTPAREN)
    .value("RIGHTPAREN", SDLK_RIGHTPAREN)
    .value("ASTERISK", SDLK_ASTERISK)
    .value("PLUS", SDLK_PLUS)
    .value("COMMA", SDLK_COMMA)
    .value("MINUS", SDLK_MINUS)
    .value("PERIOD", SDLK_PERIOD)
    .value("SLASH", SDLK_SLASH)
    .value("DIGIT0", SDLK_0)
    .value("DIGIT1", SDLK_1)
    .value("DIGIT2", SDLK_2)
    .value("DIGIT3", SDLK_3)
    .value("DIGIT4", SDLK_4)
    .value("DIGIT5", SDLK_5)
    .value("DIGIT6", SDLK_6)
    .value("DIGIT7", SDLK_7)
    .value("DIGIT8", SDLK_8)
    .value("DIGIT9", SDLK_9)
    .value("COLON", SDLK_COLON)
    .value("SEMICOLON", SDLK_SEMICOLON)
    .value("LESS", SDLK_LESS)
    .value("EQUALS", SDLK_EQUALS)
    .value("GREATER", SDLK_GREATER)
    .value("QUESTION", SDLK_QUESTION)
    .value("AT", SDLK_AT)
    .value("LEFTBRACKET", SDLK_LEFTBRACKET)
    .value("BACKSLASH", SDLK_BACKSLASH)
    .value("RIGHTBRACKET", SDLK_RIGHTBRACKET)
    .value("CARET", SDLK_CARET)
    .value("UNDERSCORE", SDLK_UNDERSCORE)
    .value("BACKQUOTE", SDLK_BACKQUOTE)
    .value("a", SDLK_a)
    .value("b", SDLK_b)
    .value("c", SDLK_c)
    .value("d", SDLK_d)
    .value("e", SDLK_e)
    .value("f", SDLK_f)
    .value("g", SDLK_g)
    .value("h", SDLK_h)
    .value("i", SDLK_i)
    .value("j", SDLK_j)
    .value("k", SDLK_k)
    .value("l", SDLK_l)
    .value("m", SDLK_m)
    .value("n", SDLK_n)
    .value("o", SDLK_o)
    .value("p", SDLK_p)
    .value("q", SDLK_q)
    .value("r", SDLK_r)
    .value("s", SDLK_s)
    .value("t", SDLK_t)
    .value("u", SDLK_u)
    .value("v", SDLK_v)
    .value("w", SDLK_w)
    .value("x", SDLK_x)
    .value("y", SDLK_y)
    .value("z", SDLK_z)
    .value("DELETE", SDLK_DELETE)
    .value("KP0", SDLK_KP0)
    .value("KP1", SDLK_KP1)
    .value("KP2", SDLK_KP2)
    .value("KP3", SDLK_KP3)
    .value("KP4", SDLK_KP4)
    .value("KP5", SDLK_KP5)
    .value("KP6", SDLK_KP6)
    .value("KP7", SDLK_KP7)
    .value("KP8", SDLK_KP8)
    .value("KP9", SDLK_KP9)
    .value("KP_PERIOD", SDLK_KP_PERIOD)
    .value("KP_DIVIDE", SDLK_KP_DIVIDE)
    .value("KP_MULTIPLY", SDLK_KP_MULTIPLY)
    .value("KP_MINUS", SDLK_KP_MINUS)
    .value("KP_PLUS", SDLK_KP_PLUS)
    .value("KP_ENTER", SDLK_KP_ENTER)
    .value("KP_EQUALS", SDLK_KP_EQUALS)
    .value("UP", SDLK_UP)
    .value("DOWN", SDLK_DOWN)
    .value("RIGHT", SDLK_RIGHT)
    .value("LEFT", SDLK_LEFT)
    .value("INSERT", SDLK_INSERT)
    .value("HOME", SDLK_HOME)
    .value("END", SDLK_END)
    .value("PAGEUP", SDLK_PAGEUP)
    .value("PAGEDOWN", SDLK_PAGEDOWN)
    .value("F1", SDLK_F1)
    .value("F2", SDLK_F2)
    .value("F3", SDLK_F3)
    .value("F4", SDLK_F4)
    .value("F5", SDLK_F5)
    .value("F6", SDLK_F6)
    .value("F7", SDLK_F7)
    .value("F8", SDLK_F8)
    .value("F9", SDLK_F9)
    .value("F10", SDLK_F10)
    .value("F11", SDLK_F11)
    .value("F12", SDLK_F12)
    .value("F13", SDLK_F13)
    .value("F14", SDLK_F14)
    .value("F15", SDLK_F15)
    .value("NUMLOCK", SDLK_NUMLOCK)
    .value("CAPSLOCK", SDLK_CAPSLOCK)
    .value("SCROLLOCK", SDLK_SCROLLOCK)
    .value("RSHIFT", SDLK_RSHIFT)
    .value("LSHIFT", SDLK_LSHIFT)
    .value("RCTRL", SDLK_RCTRL)
    .value("LCTRL", SDLK_LCTRL)
    .value("RALT", SDLK_RALT)
    .value("LALT", SDLK_LALT)
    .value("RMETA", SDLK_RMETA)
    .value("LMETA", SDLK_LMETA)
    .value("LSUPER", SDLK_LSUPER)
    .value("RSUPER", SDLK_RSUPER)
    .value("MODE", SDLK_MODE)
    .value("COMPOSE", SDLK_COMPOSE)
    .value("HELP", SDLK_HELP)
    .value("PRINT", SDLK_PRINT)
    .value("SYSREQ", SDLK_SYSREQ)
    .value("BREAK", SDLK_BREAK)
    .value("MENU", SDLK_MENU)
    .value("POWER", SDLK_POWER)
    .value("EURO", SDLK_EURO)
    .value("UNDO", SDLK_UNDO)
    ;
}

