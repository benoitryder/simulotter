#include "python/common.h"
#include "object.h"
#include "physics.h"


static btVector3 Object_getPos(const Object& o) { return btUnscale(o.getPos()); }
static void Object_setPos(Object& o, const btVector3& v) { o.setPos(btScale(v)); }
static btTransform Object_getTrans(const Object& o) { return btUnscale(o.getTrans()); }
static void Object_setTrans(Object& o, const btTransform& tr) { o.setTrans(btScale(tr)); }

static SmartPtr<OGround> OGround_init(const btVector2& size, const Color4& color) { return new OGround(btScale(size), color); }
static btVector2 OGround_getSize(const OGround& o) { return btUnscale(o.getSize()); }

static SmartPtr<OGroundSquareStart> OGroundSquareStart_init(const btVector2& size, const Color4& color, const Color4& color_t1, const Color4& color_t2) { return new OGroundSquareStart(btScale(size), color, color_t1, color_t2); }
static btScalar OGroundSquareStart_getStartSize(const OGroundSquareStart& o) { return btUnscale(o.getStartSize()); }
static void OGroundSquareStart_setStartSize(OGroundSquareStart& o, const btScalar& v) { o.setStartSize(btScale(v)); }

// SmartPtr and not const, otherwise Boost.Python cannot convert the shape.
static SmartPtr<btCollisionShape> OSimple_getShape(OSimple& o) { return o.getCollisionShape(); }
static void OSimple_setPos(OSimple& o, const py::object v)
{
  py::extract<const btVector2&> py_vec2(v);
  if(py_vec2.check()) {
    o.setPosAbove(btScale(py_vec2()));
  } else {
    o.setPos(btScale(py::extract<const btVector3&>(v)()));
  }
}


void python_export_object()
{
  py_smart_register<Object>();

  py::class_<Object, SmartPtr<Object>, boost::noncopyable>("Object", py::no_init)
      .def("addToWorld", &Object::addToWorld)
      .def("removeFromWorld", &Object::removeFromWorld)
      .add_property("pos", &Object_getPos, &Object_setPos)
      .add_property("rot", &Object::getRot, &Object::setRot)
      .add_property("trans", &Object_getTrans, &Object_setTrans)
      .add_property("physics", py::make_function(
              &Object::getPhysics, py::return_internal_reference<>()))
      ;

  py::class_<OSimple, py::bases<Object>, SmartPtr<OSimple>, boost::noncopyable>("OSimple")
      .def(py::init<btCollisionShape*, btScalar>(
              (py::arg("shape"), py::arg("mass")=0)))
      .add_property("shape", &OSimple_getShape, &OSimple::setShape)
      .add_property("mass", &OSimple::getMass, &OSimple::setMass)
      .add_property("initialized", &OSimple::isInitialized)
      .add_property("color", &OSimple::getColor, &OSimple::setColor)
      // redefine to use setPosAbove() when setting vec2
      .add_property("pos", &Object_getPos, &OSimple_setPos)
      ;

  py::class_<OGround, py::bases<OSimple>, SmartPtr<OGround>, boost::noncopyable>("OGround", py::no_init)
      .def("__init__", py::make_constructor(&OGround_init))
      .add_property("size", &OGround_getSize)
      ;

  py::class_<OGroundSquareStart, py::bases<OGround>, SmartPtr<OGroundSquareStart>, boost::noncopyable>("OGroundSquareStart", py::no_init)
      .def("__init__", py::make_constructor(&OGroundSquareStart_init))
      .add_property("start_size", &OGroundSquareStart_getStartSize, &OGroundSquareStart_setStartSize)
      ;
}

