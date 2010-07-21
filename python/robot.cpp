#include "python/common.h"
#include "robot.h"


void python_module_robot()
{
  py::class_<Robot, py::bases<Object>, SmartPtr<Robot>, boost::noncopyable>("Robot", py::no_init);

  //TODO RBasic
}


