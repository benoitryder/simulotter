/** @file
 * @brief Utilities and basic classes.
 */

#include "python/common.h"
#include <boost/python/stl_iterator.hpp>
#include "colors.h"
#include "physics.h"


static std::string Color4_str(const Color4 &c)
{
  return stringf("<rgba: %.2f %.2f %.2f %.2f>",
                 c.r(), c.g(), c.b(), c.a());
}

static const GLfloat *Color4_begin(const Color4 &c) { return (const GLfloat *)c; }
static const GLfloat *Color4_end(const Color4 &c) { return (const GLfloat *)c+4; }

// templates for shape constructors with scaling
template <class T, class A1> SmartPtr<T> Shape_init_scale(const A1 &a1) { return new T(btScale(a1)); }
template <class T, class A1, class A2> SmartPtr<T> Shape_init_scale(const A1 &a1, const A2 &a2) { return new T(btScale(a1), btScale(a2)); }


/// Compound shape constructor: list of (shape, trans) pairs.
static SmartPtr<CompoundShapeSmart> CompoundShape_init(const py::object o)
{
  SmartPtr<CompoundShapeSmart> ret_shape = new CompoundShapeSmart();

  py::stl_input_iterator<py::object> it(o), it_end;
  for( ; it != it_end; ++it ) {
    const py::object &p = *it;
    py::ssize_t n = py::len(p);
    if( n != 2 ) {
      // cast to avoid type length issues
      throw Error("invalid pair length: %u", (unsigned int)n);
    }
    btCollisionShape *sh = py::extract<btCollisionShape *>( p[0] );
    const btTransform &tr = py::extract<const btTransform &>( p[1] );
    ret_shape->addChildShape(btScale(tr), sh);
  }
  // On error, updateChildReferences is not called and pointers owned by the
  // compound shape are simply destroyed with it.
  // Shapes are still owned by the python object, thus don't have to be
  // destroyed.
  ret_shape->updateChildReferences();
  return ret_shape;
}


void python_export_utils()
{
  py::class_<Color4>("Color")
      .def(py::init<GLfloat, GLfloat, GLfloat, GLfloat>(
              (py::arg("r")=0, py::arg("g")=0, py::arg("b")=0, py::arg("a")=1)))
      .def(py::init<GLfloat>())
      .def(py::init<int, int, int, int>(
              (py::arg("r")=0, py::arg("g")=0, py::arg("b")=0, py::arg("a")=1)))
      .def(py::init<Color4>())
      .add_property("r", &Color4::r)
      .add_property("g", &Color4::g)
      .add_property("b", &Color4::b)
      .add_property("a", &Color4::a)
      .def(py::self * GLfloat())
      .def(GLfloat() * py::self)
      .def("white", &Color4::white).staticmethod("white")
      .def("black", &Color4::black).staticmethod("black")
      .def("plexi", &Color4::plexi).staticmethod("plexi")
      .def("__repr__", Color4_str)
      .def("__iter__", py::range(&Color4_begin, &Color4_end))
      ;

  // shapes (not all exported)

  py::class_<btCollisionShape, SmartPtr<btCollisionShape>, boost::noncopyable>("Shape", py::no_init);
#define EXPORT_SHAPE_CLASS(n,T,B) \
  py::class_<T, py::bases<B>, SmartPtr<T>, boost::noncopyable>(#n, py::no_init)
#define EXPORT_SHAPE_CLASS_INIT(n,T,B, ...) \
  EXPORT_SHAPE_CLASS(n,T,B).def("__init__", py::make_constructor(&Shape_init_scale<T, ## __VA_ARGS__>))

  EXPORT_SHAPE_CLASS_INIT(ShSphere,    btSphereShape,    btCollisionShape, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShCapsule,   btCapsuleShape,   btCollisionShape, btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShCapsuleX,  btCapsuleShapeX,  btCapsuleShape  , btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShCapsuleZ,  btCapsuleShapeZ,  btCapsuleShape  , btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShCone,      btConeShape,      btCollisionShape, btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShConeX,     btConeShapeX,     btConeShape     , btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShConeZ,     btConeShapeZ,     btConeShape     , btScalar, btScalar);
  EXPORT_SHAPE_CLASS_INIT(ShCylinder,  btCylinderShape,  btCollisionShape, btVector3);
  EXPORT_SHAPE_CLASS_INIT(ShCylinderX, btCylinderShapeX, btCylinderShape , btVector3);
  EXPORT_SHAPE_CLASS_INIT(ShCylinderZ, btCylinderShapeZ, btCylinderShape , btVector3);
  EXPORT_SHAPE_CLASS_INIT(ShBox,       btBoxShape,       btCollisionShape, btVector3);
  // compound shape: special class (memory handling) and special constructor
  EXPORT_SHAPE_CLASS(ShCompound, CompoundShapeSmart, btCollisionShape)
      .def("__init__", py::make_constructor(&CompoundShape_init))
      ;

#undef EXPORT_SHAPE_CLASS
#undef EXPORT_SHAPE_CLASS_INIT

}


