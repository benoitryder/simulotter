#include "python/common.h"
#include "modules/eurobot2011.h"

using namespace eurobot2011;

static const btScalar OGround2011_START_SIZE = btUnscale(OGround2011::START_SIZE);
static const btScalar OGround2011_SQUARE_SIZE = btUnscale(OGround2011::SQUARE_SIZE);



void python_export_eurobot2011()
{
  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2011);

  py::class_<OGround2011, py::bases<OGround>, SmartPtr<OGround2011>, boost::noncopyable>("OGround")
      .def_readonly("START_SIZE", OGround2011_START_SIZE)
      .def_readonly("SQUARE_SIZE", OGround2011_SQUARE_SIZE)
      ;

}

