#include "python/common.h"
#include "robot.h"


static btScalar RBasic_get_v(const RBasic& r) { return btUnscale(r.getVelocity()); }
static btScalar RBasic_get_v_max(const RBasic& r) { return btUnscale(r.v_max); }
static void RBasic_set_v_max(RBasic& r, btScalar v) { r.v_max = btScale(v); }
static btScalar RBasic_get_threshold_xy(const RBasic& r) { return btUnscale(r.threshold_xy); }
static void RBasic_set_threshold_xy(RBasic& r, btScalar v) { r.threshold_xy = btScale(v); }

static void RBasic_order_xy(RBasic& r, const btVector2& xy, bool rel) { r.order_xy(btScale(xy), rel); }
static void RBasic_order_xya(RBasic& r, const btVector2& xy, btScalar a, bool rel) { r.order_xya(btScale(xy), a, rel); }
static void RBasic_order_back(RBasic& r, btScalar d) { r.order_back(btScale(d)); }



void python_export_robot()
{
  py::class_<Robot, py::bases<Object>, SmartPtr<Robot>, boost::noncopyable>("Robot", py::no_init);

  py::class_<RBasic, py::bases<Robot>, SmartPtr<RBasic>, boost::noncopyable>("RBasic", py::no_init)
      .def(py::init<btCollisionShape*, btScalar>(
              (py::arg("shape"), py::arg("mass"))))
      .add_property("color", &RBasic::getColor, &RBasic::setColor)
      .def("asserv", &RBasic::asserv)
      .add_property("a", &RBasic::getAngle)
      .add_property("v", &RBasic_get_v)
      .add_property("av", &RBasic::getAngularVelocity)
      .add_property("v_max", &RBasic_get_v_max, &RBasic_set_v_max)
      .def_readwrite("av_max", &RBasic::av_max)
      .add_property("threshold_xy", &RBasic_get_threshold_xy, &RBasic_set_threshold_xy)
      .def_readwrite("threshold_a", &RBasic::threshold_a)
      .def("order_xy", &RBasic_order_xy, ( py::arg("xy"), py::arg("rel")=false ))
      .def("order_a", &RBasic::order_a, ( py::arg("a"), py::arg("rel")=false ))
      .def("order_xya", &RBasic_order_xya, ( py::arg("xy"), py::arg("a"), py::arg("rel")=false ))
      .def("order_back", &RBasic_order_back)
      .def("order_stop", &RBasic::order_stop)
      .def("is_waiting", &RBasic::is_waiting)
      ;
}


