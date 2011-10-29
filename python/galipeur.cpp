#include "python/common.h"
#include <boost/python/stl_iterator.hpp>
#include "galipeur.h"


static btVector3 Galipeur_getPos(const Galipeur &o) { return btUnscale(o.getPos()); }
static void Galipeur_setPos(Galipeur &o, const py::object v)
{
  py::extract<const btVector2 &> py_vec2(v);
  if( py_vec2.check() ) {
    o.setPosAbove(btScale(py_vec2()));
  } else {
    o.setPos(btScale(py::extract<const btVector3 &>(v)()));
  }
}

static inline btVector2 Galipeur_get_v(const Galipeur &g) { return btUnscale(g.getVelocity()); }

static void Galipeur_order_xy(Galipeur &g, const btVector2 &xy, bool rel) { g.order_xy(btScale(xy), rel); }
static void Galipeur_order_xya(Galipeur &g, const btVector2 &xy, btScalar a, bool rel) { g.order_xya(btScale(xy), a, rel); }
static void Galipeur_order_trajectory(Galipeur &g, const py::object o)
{
  Galipeur::CheckPoints checkpoints;
  py::stl_input_iterator<py::object> it(o), it_end;
  for( ; it != it_end; ++it ) {
    checkpoints.push_back( py::extract<btVector2>(*it) );
  }
  g.order_trajectory(checkpoints);
}

static void Galipeur_set_speed_xy(Galipeur &g, btScalar v, btScalar a) { g.set_speed_xy(btScale(v), btScale(a)); }
static void Galipeur_set_speed_steering(Galipeur &g, btScalar v, btScalar a) { g.set_speed_steering(btScale(v), btScale(a)); }
static void Galipeur_set_speed_stop(Galipeur &g, btScalar v, btScalar a) { g.set_speed_stop(btScale(v), btScale(a)); }
static void Galipeur_set_threshold_stop(Galipeur &g, btScalar r, btScalar l) { g.set_threshold_stop(btScale(r), l); }
static void Galipeur_set_threshold_steering(Galipeur &g, btScalar t) { g.set_threshold_steering(btScale(t)); }


void python_export_galipeur()
{
  static const btScalar Galipeur_Z_MASS = btUnscale(Galipeur::Z_MASS);
  static const btScalar Galipeur_GROUND_CLEARANCE = btUnscale(Galipeur::GROUND_CLEARANCE);
  static const btScalar Galipeur_HEIGHT = btUnscale(Galipeur::HEIGHT);
  static const btScalar Galipeur_SIDE = btUnscale(Galipeur::SIDE);
  static const btScalar Galipeur_W_BLOCK = btUnscale(Galipeur::W_BLOCK);
  static const btScalar Galipeur_R_WHEEL = btUnscale(Galipeur::R_WHEEL);
  static const btScalar Galipeur_H_WHEEL = btUnscale(Galipeur::H_WHEEL);
  static const btScalar Galipeur_D_SIDE = btUnscale(Galipeur::D_SIDE);
  static const btScalar Galipeur_D_WHEEL = btUnscale(Galipeur::D_WHEEL);
  static const btScalar Galipeur_RADIUS = btUnscale(Galipeur::RADIUS);


  py::class_<Galipeur, py::bases<Robot>, SmartPtr<Galipeur>, boost::noncopyable>("Galipeur", py::no_init)
      .def(py::init<btScalar>())
      .add_property("color", &Galipeur::getColor, &Galipeur::setColor)
      // redefine to use setPosAbove() when setting vec2
      .add_property("pos", &Galipeur_getPos, &Galipeur_setPos)
      .def("asserv", &Galipeur::asserv)
      .add_property("a", &Galipeur::getAngle)
      .add_property("v", &Galipeur_get_v)
      .add_property("av", &Galipeur::getAngularVelocity)
      .def("order_xy", &Galipeur_order_xy, ( py::arg("xy"), py::arg("rel")=false ))
      .def("order_a", &Galipeur::order_a, ( py::arg("a"), py::arg("rel")=false ))
      .def("order_xya", &Galipeur_order_xya, ( py::arg("xy"), py::arg("a"), py::arg("rel")=false ))
      .def("order_stop", &Galipeur::order_stop)
      .def("order_trajectory", &Galipeur_order_trajectory)
      .def("order_xy_done", &Galipeur::order_xy_done)
      .def("order_a_done", &Galipeur::order_a_done)
      .def("is_waiting", &Galipeur::is_waiting)
      .def("current_checkpoint", &Galipeur::current_checkpoint)
      // configuration
      .def("set_speed_xy", &Galipeur_set_speed_xy)
      .def("set_speed_a", &Galipeur::set_speed_a)
      .def("set_speed_steering", &Galipeur_set_speed_steering)
      .def("set_speed_stop", &Galipeur_set_speed_stop)
      .def("set_threshold_stop", &Galipeur_set_threshold_stop)
      .def("set_threshold_steering", &Galipeur_set_threshold_steering)
      // statics
      .def_readonly("Z_MASS", Galipeur_Z_MASS)
      .def_readonly("GROUND_CLEARANCE", Galipeur_GROUND_CLEARANCE)
      .def_readonly("ANGLE_OFFSET", Galipeur::ANGLE_OFFSET)
      .def_readonly("HEIGHT", Galipeur_HEIGHT)
      .def_readonly("SIDE", Galipeur_SIDE)
      .def_readonly("W_BLOCK", Galipeur_W_BLOCK)
      .def_readonly("R_WHEEL", Galipeur_R_WHEEL)
      .def_readonly("H_WHEEL", Galipeur_H_WHEEL)
      .def_readonly("D_SIDE", Galipeur_D_SIDE)
      .def_readonly("D_WHEEL", Galipeur_D_WHEEL)
      .def_readonly("A_SIDE", Galipeur::A_SIDE)
      .def_readonly("A_WHEEL", Galipeur::A_WHEEL)
      .def_readonly("RADIUS", Galipeur_RADIUS)
      ;
}

