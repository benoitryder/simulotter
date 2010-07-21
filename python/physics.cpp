#include "python/common.h"
#include "physics.h"


void python_module_physics()
{
  //TODO configuration values
  py::class_<Physics, SmartPtr<Physics>, boost::noncopyable>("Physics")
      .def("step", &Physics::step)
      .add_property("step_dt", &Physics::getStepDt)
      .add_property("time", &Physics::getTime)
      ;
}


