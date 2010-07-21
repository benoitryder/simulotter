#include <boost/python/module.hpp>


void python_module_utils();
void python_module_maths();
void python_module_display();
void python_module_physics();
void python_module_object();
void python_module_robot();


BOOST_PYTHON_MODULE(simulotter)
{
  python_module_utils();
  python_module_maths();
  python_module_display();
  python_module_physics();
  python_module_object();
  python_module_robot();
}


