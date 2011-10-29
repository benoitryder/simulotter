#include "python/common.h"
#include "modules/eurobot2011.h"

using namespace eurobot2011;

static py::object Galipeur2011_get_arms(const Galipeur2011 &o)
{
  //XXX assume a fixed arm count to be able to use make_tuple (much simpler)
#if GALIPEUR2011_ARM_NB != 2
#error "Unexpected Galipeur2011 arm count."
#endif
  return py::make_tuple(
      py::pointer_wrapper<Galipeur2011::PawnArm *>(o.getArms()[0]),
      py::pointer_wrapper<Galipeur2011::PawnArm *>(o.getArms()[1])
      );
}


void python_export_eurobot2011()
{
  static const btVector2 OGround2011_SIZE = btUnscale(OGround2011::SIZE);
  static const btScalar OGround2011_START_SIZE = btUnscale(OGround2011::START_SIZE);
  static const btScalar OGround2011_SQUARE_SIZE = btUnscale(OGround2011::SQUARE_SIZE);

  static const btScalar MagnetPawn_RADIUS = btUnscale(MagnetPawn::RADIUS);
  static const btScalar MagnetPawn_HEIGHT = btUnscale(MagnetPawn::HEIGHT);

  static const btScalar Galipeur2011_ARM_RADIUS = btUnscale(Galipeur2011::PawnArm::RADIUS);
  static const btScalar Galipeur2011_ARM_LENGTH = btUnscale(Galipeur2011::PawnArm::LENGTH);


  SIMULOTTER_PYTHON_SUBMODULE(_eurobot2011);

  py::class_<OGround2011, py::bases<OGround>, SmartPtr<OGround2011>, boost::noncopyable>("OGround")
      .def_readonly("SIZE", OGround2011_SIZE)
      .def_readonly("START_SIZE", OGround2011_START_SIZE)
      .def_readonly("SQUARE_SIZE", OGround2011_SQUARE_SIZE)
      ;

  py::class_<MagnetPawn, py::bases<OSimple>, SmartPtr<MagnetPawn>, boost::noncopyable>("_MagnetPawn", py::init<btCollisionShape *, btScalar>())
      // statics
      .def_readonly("RADIUS", MagnetPawn_RADIUS)
      .def_readonly("HEIGHT", MagnetPawn_HEIGHT)
      ;

  py::class_<Galipeur2011, py::bases<Galipeur>, SmartPtr<Galipeur2011>, boost::noncopyable>("Galipeur", py::no_init)
      .def(py::init<btScalar>())
      .add_property("arms", py::make_function(
              &Galipeur2011_get_arms,
              py::with_custodian_and_ward_postcall<1,0>()
              ))
      // configuration
      .def("set_arm_av", &Galipeur2011::set_arm_av)
      // statics
      .def_readonly("ARM_RADIUS", Galipeur2011_ARM_RADIUS)
      .def_readonly("ARM_LENGTH", Galipeur2011_ARM_LENGTH)
      .def_readonly("ARM_ANGLE_MIN", &Galipeur2011::PawnArm::ANGLE_MIN)
      .def_readonly("ARM_ANGLE_MAX", &Galipeur2011::PawnArm::ANGLE_MAX)
      ;

  py::class_<Galipeur2011::PawnArm, Galipeur2011::PawnArm*, boost::noncopyable>("PawnArm", py::no_init)
      .add_property("angle", &Galipeur2011::PawnArm::angle)
      .def("up", &Galipeur2011::PawnArm::raise) // cannot use 'raise', it's a keyword
      .def("down", &Galipeur2011::PawnArm::lower)
      .def("grab", &Galipeur2011::PawnArm::grab)
      .def("release", &Galipeur2011::PawnArm::release)
      ;

}

