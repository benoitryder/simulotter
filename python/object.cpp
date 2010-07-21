#include "python/common.h"
#include "object.h"
#include "physics.h"


static inline btVector3 Object_getPos(const Object &o) { return btUnscale(o.getPos()); }
static inline void Object_setPos(Object &o, const btVector3 &v) { o.setPos(btScale(v)); }
static inline btTransform Object_getTrans(const Object &o) { return btUnscale(o.getTrans()); }
static inline void Object_setTrans(Object &o, const btTransform &tr) { o.setTrans(btScale(tr)); }

static inline btMatrix3x3 Object_getRot(const Object &o) { return o.getRot(); }

static inline void OSimple_setPosAbove(OSimple &o, const btVector2 &v) { o.setPosAbove(btScale(v)); }


void python_module_object()
{
  py::class_<Object, SmartPtr<Object>, boost::noncopyable>("Object", py::no_init)
      .def("addToWorld", &Object::addToWorld)
      .def("removeFromWorld", &Object::removeFromWorld)
      .add_property("pos", &Object_getPos, &Object_setPos)
      .add_property("rot", &Object_getRot, &Object::setRot)
      .add_property("trans", &Object_getTrans, &Object_setTrans)
      ;

  py::class_<OSimple, py::bases<Object>, SmartPtr<OSimple>, boost::noncopyable>("OSimple")
      //TODO set shape, mass, etc. in constructor?
      .def("setShape", &OSimple::setShape)
      .def("setMass", &OSimple::setMass)
      .def("isInitialized", &OSimple::isInitialized)
      .def("setColor", &OSimple::setColor)
      .def("setPosAbove", &OSimple_setPosAbove)
      ;

  py::class_<OGround, py::bases<OSimple>, SmartPtr<OGround>, boost::noncopyable>(
      "OGround", py::init<Color4, Color4, Color4>())
      ;
}

