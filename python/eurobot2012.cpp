#include "python/common.h"
#include "modules/eurobot2012.h"

using namespace eurobot2012;

static const btVector2 OGround2012_SIZE = btUnscale(OGround2012::SIZE);
static const btScalar OGround2012_START_SIZE = btUnscale(OGround2012::START_SIZE);

static const btVector3 OBullion_SIZE = btUnscale(OBullion::SIZE);

static const btScalar OCoin_DISC_HEIGHT = btUnscale(OCoin::DISC_HEIGHT);
static const btScalar OCoin_RADIUS = btUnscale(OCoin::RADIUS);
static const btScalar OCoin_INNER_RADIUS = btUnscale(OCoin::INNER_RADIUS);
static const btScalar OCoin_CUBE_SIZE = btUnscale(OCoin::CUBE_SIZE);


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

  py::class_<OCoin, py::bases<OSimple>, SmartPtr<OCoin>, boost::noncopyable>("OCoin", py::init<bool>())
      .def_readonly("DISC_HEIGHT", OCoin_DISC_HEIGHT)
      .def_readonly("RADIUS", OCoin_RADIUS)
      .def_readonly("INNER_RADIUS", OCoin_INNER_RADIUS)
      .def_readonly("CUBE_SIZE", OCoin_CUBE_SIZE)
      .def_readonly("MASS", OCoin::MASS)
      ;
}

