#include "python/common.h"


void python_export_utils();
void python_export_maths();
void python_export_display();
void python_export_physics();
void python_export_object();
void python_export_robot();
void python_export_galipeur();
void python_export_eurobot2009();
void python_export_eurobot2010();
void python_export_eurobot2011();


// indirection to allow module name defined by a macro
#define XBOOST_PYTHON_MODULE(name) BOOST_PYTHON_MODULE(name)

XBOOST_PYTHON_MODULE(SIMULOTTER_MODULE_NAME)
{
  py::object package = py::scope();
  package.attr("__path__") = SIMULOTTER_MODULE_NAME_STR;

  // core
  python_export_utils();
  python_export_maths();
  python_export_display();
  python_export_physics();
  python_export_object();
  python_export_robot();
  python_export_galipeur();

  // sub modules
  python_export_eurobot2009();
  python_export_eurobot2010();
  python_export_eurobot2011();
}


