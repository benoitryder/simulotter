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

static btScalar Display_get_draw_epsilon() { return btUnscale(Display::draw_epsilon); }
static void Display_set_draw_epsilon(btScalar v) { Display::draw_epsilon = btScale(v); }

static py::tuple Display_get_cam_pos(const Display &d)
{
  btVector3 eye, target;
  d.getCameraPos(eye,target);
  return py::make_tuple(eye,target);
}

static btVector3 CamPoint_get_cart(const Display::CameraPoint *p) { return btUnscale(p->cart); }
static void CamPoint_set_cart(Display::CameraPoint *p, const btVector3 &v) { p->cart = btScale(v); }
static btSpheric3 CamPoint_get_spheric(const Display::CameraPoint *p) { return btUnscale(p->spheric); }
static void CamPoint_set_spheric(Display::CameraPoint *p, const btSpheric3 &v) { p->spheric = btScale(v); }
static SmartPtr<Object> CamPoint_get_obj(const Display::CameraPoint *p) { return p->obj; }
static void CamPoint_set_obj(Display::CameraPoint *p, Object *o) { p->obj = o; }


void python_export_display()
{
  py::scope in_Display = py::class_<Display, SmartPtr<Display>, boost::noncopyable>("Display")
      .def("run", &Display::run)
      .add_property("physics",
                    py::make_function(&Display::getPhysics, py::return_internal_reference<>()),
                    &Display::setPhysics)
      .def("update", &Display::update)
      .def("resize", &Display_resize,
           ( py::arg("width"), py::arg("height"), py::arg("mode")=py::object() ))
      .def("screenshot", &Display::savePNGScreenshot)
      // dynamic configuration
      .def_readwrite("time_scale", &Display::time_scale)
      .def_readwrite("fps", &Display::fps)
      .def_readwrite("bg_color", &Display::bg_color)
      .def_readwrite("camera_step_angle", &Display::camera_step_angle)
      .def_readwrite("camera_step_linear", &Display::camera_step_linear)
      .def_readwrite("camera_mouse_coef", &Display::camera_mouse_coef)
      .def_readwrite("perspective_fov", &Display::perspective_fov)
      .def_readwrite("perspective_near", &Display::perspective_near)
      .def_readwrite("perspective_far", &Display::perspective_far)
      .add_property("cam_mode", &Display::getCameraMode, &Display::setCameraMode)
      .add_property("cam_pos", &Display_get_cam_pos)
      .add_property("cam_eye", py::make_function(&Display::getCameraEye, py::return_internal_reference<>()))
      .add_property("cam_target", py::make_function(&Display::getCameraTarget, py::return_internal_reference<>()))
      // statics
      .add_static_property("draw_epsilon", &Display_get_draw_epsilon, &Display_set_draw_epsilon)
      .def_readwrite("draw_div", &Display::draw_div)
      .def_readwrite("antialias", &Display::antialias)
      ;

  py::class_<Display::CameraPoint, boost::noncopyable>("CamPoint", py::no_init)
      .add_property("cart", &CamPoint_get_cart, &CamPoint_set_cart)
      .add_property("spheric", &CamPoint_get_spheric, &CamPoint_set_spheric)
      // break strict-aliasing rules, use property instead
      //.def_readwrite("obj", &Display::CameraPoint::obj)
      .add_property("obj", &CamPoint_get_obj, &CamPoint_set_obj)
      ;

  py::enum_<Display::CamMode>("CamMode")
      .value("EYE_FIXED", Display::CAM_EYE_FIXED)
      .value("EYE_REL", Display::CAM_EYE_REL)
      .value("EYE_OBJECT", Display::CAM_EYE_OBJECT)
      .value("EYE_MASK", Display::CAM_EYE_MASK)
      .value("TARGET_FIXED", Display::CAM_TARGET_FIXED)
      .value("TARGET_REL", Display::CAM_TARGET_REL)
      .value("TARGET_OBJECT", Display::CAM_TARGET_OBJECT)
      .value("TARGET_MASK", Display::CAM_TARGET_MASK)
      .value("FREE", Display::CAM_FREE)
      .value("FIXED", Display::CAM_FIXED)
      .value("FOLLOW", Display::CAM_FOLLOW)
      .value("ONBOARD", Display::CAM_ONBOARD)
      .value("LOOK", Display::CAM_LOOK)
      .export_values()
      ;
}


