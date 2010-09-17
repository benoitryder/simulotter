#include "python/common.h"
#include "modules/eurobot2010.h"

using namespace eurobot2010;

static const btScalar ORaisedZone_WIDTH = btUnscale(ORaisedZone::WIDTH);
static const btScalar ORaisedZone_HEIGHT = btUnscale(ORaisedZone::HEIGHT);
static const btScalar ORaisedZone_BOTTOM_LENGTH = btUnscale(ORaisedZone::BOTTOM_LENGTH);
static const btScalar ORaisedZone_TOP_LENGTH = btUnscale(ORaisedZone::TOP_LENGTH);
static const btScalar ORaisedZone_STRIP_LENGTH = btUnscale(ORaisedZone::STRIP_LENGTH);
static const btScalar ORaisedZone_WALL_WIDTH = btUnscale(ORaisedZone::WALL_WIDTH);
static const btScalar ORaisedZone_WALL_HEIGHT = btUnscale(ORaisedZone::WALL_HEIGHT);
static const btScalar ORaisedZone_WALL_BOTTOM_LENGTH = btUnscale(ORaisedZone::WALL_BOTTOM_LENGTH);
static const btScalar ORaisedZone_WALL_TOP_LENGTH = btUnscale(ORaisedZone::WALL_TOP_LENGTH);

static void OCorn_plant(OCorn &o, btScalar x, btScalar y) { o.plant(btScale(x), btScale(y)); }



void python_export_eurobot2010()
{
  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2010);

  py::class_<ORaisedZone, py::bases<OSimple>, SmartPtr<ORaisedZone>, boost::noncopyable>("ORaisedZone")
      .def_readonly("WIDTH", ORaisedZone_WIDTH)
      .def_readonly("HEIGHT", ORaisedZone_HEIGHT)
      .def_readonly("BOTTOM_LENGTH", ORaisedZone_BOTTOM_LENGTH)
      .def_readonly("TOP_LENGTH", ORaisedZone_TOP_LENGTH)
      .def_readonly("STRIP_LENGTH", ORaisedZone_STRIP_LENGTH)
      .def_readonly("WALL_WIDTH", ORaisedZone_WALL_WIDTH)
      .def_readonly("WALL_HEIGHT", ORaisedZone_WALL_HEIGHT)
      .def_readonly("WALL_BOTTOM_LENGTH", ORaisedZone_WALL_BOTTOM_LENGTH)
      .def_readonly("WALL_TOP_LENGTH", ORaisedZone_WALL_TOP_LENGTH)
      ;

  py::class_<OCorn, py::bases<OSimple>, SmartPtr<OCorn>, boost::noncopyable>("OCorn")
      .def("plant", &OCorn_plant)
      .def("uproot", &OCorn::uproot)
      ;

  py::class_<OCornFake, py::bases<OSimple>, SmartPtr<OCornFake>, boost::noncopyable>("OCornFake");
  py::class_<OTomato, py::bases<OSimple>, SmartPtr<OTomato>, boost::noncopyable>("OTomato");
  py::class_<OOrange, py::bases<OSimple>, SmartPtr<OOrange>, boost::noncopyable>("OOrange");
}

