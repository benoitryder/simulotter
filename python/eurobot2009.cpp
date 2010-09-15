#include "python/common.h"
#include "modules/eurobot2009.h"

using namespace eurobot2009;


static const btScalar ODispenser_HEIGHT = btUnscale(ODispenser::HEIGHT);
static const btScalar ODispenser_RADIUS = btUnscale(ODispenser::RADIUS);

static void ODispenser_setPos(ODispenser &o, const btVector3 &v, int side) { o.setPos(btScale(v), side); }
static void ODispenser_fill(ODispenser &o, Object *oo, btScalar z) { o.fill(oo, btScale(z)); }
static void OLintelStorage_setPos(OLintelStorage &o, btScalar d, int side) { o.setPos(btScale(d), side); }

static btScalar Galipeur_get_pachev_pos(const Galipeur2009 &g) { return btUnscale(g.get_pachev_pos()); }
static void Galipeur_order_pachev_move(Galipeur2009 &g, btScalar h) { g.order_pachev_move(btScale(h)); }
static void Galipeur_set_pachev_v(Galipeur2009 &g, btScalar h) { g.set_pachev_v(btScale(h)); }
static void Galipeur_set_threshold_pachev(Galipeur2009 &g, btScalar t) { g.set_threshold_pachev(btScale(t)); }
static btScalar Galipeur_get_pachev_eject_speed() { return btUnscale(Galipeur2009::pachev_eject_speed); }
static void Galipeur_set_pachev_eject_speed(btScalar v) { Galipeur2009::pachev_eject_speed = btScale(v); }


void python_export_eurobot2009()
{
  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2009);

  py::class_<OColElem, py::bases<OSimple>, SmartPtr<OColElem>, boost::noncopyable>("OColElem");
  py::class_<OLintel, py::bases<OSimple>, SmartPtr<OLintel>, boost::noncopyable>("OLintel");

  py::class_<ODispenser, py::bases<OSimple>, SmartPtr<ODispenser>, boost::noncopyable>("ODispenser")
      .def("setPos", &ODispenser_setPos)
      .def("fill", &ODispenser_fill)
      .def_readonly("RADIUS", ODispenser_RADIUS)
      .def_readonly("HEIGHT", ODispenser_HEIGHT)
      ;

  py::class_<OLintelStorage, py::bases<OSimple>, SmartPtr<OLintelStorage>, boost::noncopyable>("OLintelStorage")
      .def("setPos", &OLintelStorage_setPos)
      .def("fill", &OLintelStorage::fill)
      ;

  py::class_<Galipeur2009, py::bases<Galipeur>, SmartPtr<Galipeur2009>, boost::noncopyable>("Galipeur", py::no_init)
      .def(py::init<btScalar>())
      .add_property("pachev_pos", &Galipeur_get_pachev_pos)
      .def("order_pachev_move", &Galipeur_order_pachev_move)
      .def("order_pachev_release", &Galipeur2009::order_pachev_release)
      .def("order_pachev_grab", &Galipeur2009::order_pachev_grab)
      .def("order_pachev_eject", &Galipeur2009::order_pachev_eject)
      .def("set_pachev_v", &Galipeur_set_pachev_v)
      .def("set_threshold_pachev", &Galipeur_set_threshold_pachev)
      .add_static_property("pachev_eject_speed", &Galipeur_get_pachev_eject_speed, &Galipeur_set_pachev_eject_speed)
      ;
}


