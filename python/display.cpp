#include "python/common.h"
#include "display.h"
#include "physics.h"
#include "object.h"


static void Display_resize(Display &d, int width, int height, py::object mode)
{
  int mode_cpp;
  if( mode.ptr() == py::object().ptr() ) {
    mode_cpp = -1; // none: use current state
  } else {
    mode_cpp = mode ? 1 : 0;
  }
  d.resize(width, height, mode_cpp);
}
static py::object Display_get_screen_size(const Display &d)
{
  return py::make_tuple( d.getScreenWidth(), d.getScreenHeight() );
}

static btScalar Display_get_draw_epsilon() { return btUnscale(Display::draw_epsilon); }
static void Display_set_draw_epsilon(btScalar v) { Display::draw_epsilon = btScale(v); }

static btTransform Camera_get_trans(const Display::Camera &o) { return btUnscale(o.trans); }
static void Camera_set_trans(Display::Camera &o, const btTransform &tr) { o.trans = btScale(tr); }
static SmartPtr<Object> Camera_get_obj(const Display::Camera &o) { return o.obj; }
static void Camera_set_obj(Display::Camera &o, const SmartPtr<Object> &obj) { o.obj = obj; }
static float Camera_get_z_near(const Display::Camera &o) { return btUnscale(o.z_near); }
static void Camera_set_z_near(Display::Camera &o, float v) { o.z_near = btScale(v); }
static float Camera_get_z_far(const Display::Camera &o) { return btUnscale(o.z_far); }
static void Camera_set_z_far(Display::Camera &o, float v) { o.z_far = btScale(v); }


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
      // dynamic configuration
      .def_readwrite("time_scale", &Display::time_scale)
      .def_readwrite("fps", &Display::fps)
      .def_readwrite("bg_color", &Display::bg_color)
      .add_property("camera", py::make_getter(&Display::camera, py::return_internal_reference<>()))
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
      ;
}


