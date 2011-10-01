#include "python/common.h"
#include "modules/eurobot2012.h"

using namespace eurobot2012;

static const btVector2 OGround2012_SIZE = btUnscale(OGround2012::SIZE);
static const btScalar OGround2012_START_SIZE = btUnscale(OGround2012::START_SIZE);

static const btVector3 OBullion_SIZE = btUnscale(OBullion::SIZE);


void python_export_eurobot2012()
{
  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2012);

  py::class_<OGround2012, py::bases<OGround>, SmartPtr<OGround2012>, boost::noncopyable>("OGround")
      .def_readonly("SIZE", OGround2012_SIZE)
      .def_readonly("START_SIZE", OGround2012_START_SIZE)
      ;

  py::class_<OBullion, py::bases<OSimple>, SmartPtr<OBullion>, boost::noncopyable>("OBullion")
      .def_readonly("SIZE", OBullion_SIZE)
      .def_readonly("MASS", OBullion::MASS)
      .def_readonly("A_SLOPE", OBullion::A_SLOPE)
      ;

}

