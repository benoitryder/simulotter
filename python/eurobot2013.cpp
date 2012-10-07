#include "python/common.h"
#include "modules/eurobot2013.h"

using namespace eurobot2013;


void python_export_eurobot2013()
{
  static const btVector2 OGround2013_SIZE = btUnscale(OGround2013::SIZE);
  static const btScalar OGround2013_SQUARE_SIZE = btUnscale(OGround2013::SQUARE_SIZE);

  static const btScalar OCake_LEVEL_HEIGHT = btUnscale(OCake::LEVEL_HEIGHT);
  static const btScalar OCake_LEVEL_RADIUS = btUnscale(OCake::LEVEL_RADIUS);
  static const btScalar OCake_BASE_RADIUS = btUnscale(OCake::BASE_RADIUS);
  static const btScalar OCake_BASKET_HEIGHT = btUnscale(OCake::BASKET_HEIGHT);
  static const btScalar OCake_BASKET_RADIUS = btUnscale(OCake::BASKET_RADIUS);
  static const btScalar OCake_BASKET_WIDTH = btUnscale(OCake::BASKET_WIDTH);

  static const btVector3 OGiftSupport_SIZE = btUnscale(OGiftSupport::SIZE);

  static const btScalar OGlass_HEIGHT = btUnscale(OGlass::HEIGHT);
  static const btScalar OGlass_RADIUS = btUnscale(OGlass::RADIUS);
  static const btScalar OGlass_INNER_RADIUS = btUnscale(OGlass::INNER_RADIUS);
  static const btScalar OGlass_BOTTOM_HEIGHT = btUnscale(OGlass::BOTTOM_HEIGHT);

  static const btScalar OCandle_HEIGHT = btUnscale(OCandle::HEIGHT);
  static const btScalar OCandle_RADIUS = btUnscale(OCandle::RADIUS);

  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2013);

  py::class_<OGround2013, py::bases<OGround>, SmartPtr<OGround2013>, boost::noncopyable>("OGround")
      .def_readonly("SIZE", OGround2013_SIZE)
      .def_readonly("SQUARE_SIZE", OGround2013_SQUARE_SIZE)
      ;

  py::class_<OCake, py::bases<OSimple>, SmartPtr<OCake>, boost::noncopyable>("OCake")
      .def_readonly("LEVEL_HEIGHT", OCake_LEVEL_HEIGHT)
      .def_readonly("LEVEL_RADIUS", OCake_LEVEL_RADIUS)
      .def_readonly("BASE_RADIUS", OCake_BASE_RADIUS)
      .def_readonly("BASKET_HEIGHT", OCake_BASKET_HEIGHT)
      .def_readonly("BASKET_RADIUS", OCake_BASKET_RADIUS)
      .def_readonly("BASKET_WIDTH", OCake_BASKET_WIDTH)
      ;

  py::class_<OGiftSupport, py::bases<OSimple>, SmartPtr<OGiftSupport>, boost::noncopyable>("OGiftSupport")
      .def_readonly("SIZE", OGiftSupport_SIZE)
      ;

  py::class_<OGlass, py::bases<OSimple>, SmartPtr<OGlass>, boost::noncopyable>("OGlass")
      .def_readonly("SIZE", OGiftSupport_SIZE)
      .def_readonly("HEIGHT", OGlass_HEIGHT)
      .def_readonly("RADIUS", OGlass_RADIUS)
      .def_readonly("INNER_RADIUS", OGlass_INNER_RADIUS)
      .def_readonly("BOTTOM_HEIGHT", OGlass_BOTTOM_HEIGHT)
      ;

  py::class_<OCandle, py::bases<OSimple>, SmartPtr<OCandle>, boost::noncopyable>("OCandle")
      .def_readonly("HEIGHT", OCandle_HEIGHT)
      .def_readonly("RADIUS", OCandle_RADIUS)
      ;

}

