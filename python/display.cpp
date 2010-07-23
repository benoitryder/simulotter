#include "python/common.h"
#include "display.h"
#include "physics.h"


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


void python_module_display()
{
  //TODO configuration values
  py::class_<Display, SmartPtr<Display>, boost::noncopyable>("Display")
      .def("run", &Display::run)
      .add_property("physics",
                    py::make_function(&Display::getPhysics, py::return_internal_reference<>()),
                    &Display::setPhysics)
      .def("update", &Display::update)
      .def("resize", &Display_resize,
           ( py::arg("width"), py::arg("height"), py::arg("mode")=py::object() ))
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
      // constants
      .def_readonly("DRAW_EPSILON", &Display::DRAW_EPSILON)
      .def_readonly("DRAW_DIV", &Display::DRAW_DIV)
      ;
}


