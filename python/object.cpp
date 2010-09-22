#include "python/common.h"
#include "object.h"
#include "physics.h"


static inline btVector3 Object_getPos(const Object &o) { return btUnscale(o.getPos()); }
static inline void Object_setPos(Object &o, const btVector3 &v) { o.setPos(btScale(v)); }
static inline btTransform Object_getTrans(const Object &o) { return btUnscale(o.getTrans()); }
static inline void Object_setTrans(Object &o, const btTransform &tr) { o.setTrans(btScale(tr)); }

static inline btMatrix3x3 Object_getRot(const Object &o) { return o.getRot(); }

static inline void OSimple_setPos(OSimple &o, const py::object v)
{
  py::extract<const btVector2 &> py_vec2(v);
  if( py_vec2.check() ) {
    o.setPosAbove(btScale(py_vec2()));
  } else {
    o.setPos(btScale(py::extract<const btVector3 &>(v)()));
  }
}


void python_export_object()
{
  py::class_<Object, SmartPtr<Object>, boost::noncopyable>("Object", py::no_init)
      .def("addToWorld", &Object::addToWorld)
      .def("removeFromWorld", &Object::removeFromWorld)
      .add_property("pos", &Object_getPos, &Object_setPos)
      .add_property("rot", &Object_getRot, &Object::setRot)
      .add_property("trans", &Object_getTrans, &Object_setTrans)
      .add_property("physics", 
                    py::make_function(&Object::getPhysics, py::return_internal_reference<>()))
      ;

  py::class_<OSimple, py::bases<Object>, SmartPtr<OSimple>, boost::noncopyable>("OSimple")
      .def(py::init<btCollisionShape *, btScalar>(
              (py::arg("shape"), py::arg("mass")=0)))
      .def("setShape", &OSimple::setShape)
      .def("setMass", &OSimple::setMass)
      .def("isInitialized", &OSimple::isInitialized)
      .add_property("color", &OSimple::getColor, &OSimple::setColor)
      // redefine to use setPosAbove() when setting vec2
      .add_property("pos", &Object_getPos, &OSimple_setPos)
      ;

  py::class_<OGround, py::bases<OSimple>, SmartPtr<OGround>, boost::noncopyable>(
      "OGround", py::init<Color4, Color4, Color4>())
      .def_readonly("SIZE_START", OGround::SIZE_START)
      ;
}

