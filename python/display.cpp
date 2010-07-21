#include "python/common.h"
#include "display.h"
#include "physics.h"


void python_module_display()
{
  //TODO configuration values
  py::class_<Display, SmartPtr<Display>, boost::noncopyable>("Display")
      .def("setPhysics", &Display::setPhysics)
      .def("init", &Display::init)
      .def("run", &Display::run)
      ;
}


