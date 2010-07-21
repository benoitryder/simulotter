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


/// Compound shape constructor: list of (shape, trans) pairs.
static SmartPtr<CompoundShapeSmart> CompoundShape_init(const py::object o)
{
  SmartPtr<CompoundShapeSmart> ret_shape = new CompoundShapeSmart();

  py::stl_input_iterator<py::object> it(o), it_end;
  for( ; it != it_end; ++it ) {
    py::object p = *it;
    py::ssize_t n = py::len(p);
    if( n != 2 ) {
      // cast to avoid type length issues
      throw Error("invalid pair length: %u", (unsigned int)n);
    }
    btCollisionShape *sh = py::extract<btCollisionShape *>( p[0] );
    const btTransform &tr = py::extract<const btTransform &>( p[1] );
    ret_shape->addChildShape(tr, sh);
  }
  // On error, updateChildReferences is not called and pointers owned by the
  // compound shape are simply destroyed with it.
  // Shapes are still owned by the python object, thus don't have to be
  // destroyed.
  ret_shape->updateChildReferences();
  return ret_shape;
}


void python_module_utils()
{
  py::class_<Color4>("Color4")
      .def(py::init<GLfloat, GLfloat, GLfloat, py::optional<GLfloat> >())
      .def(py::init<GLfloat>())
      .def(py::init<int, int, int, py::optional<int> >())
      .def(py::init<Color4>())
      .add_property("r", &Color4::r)
      .add_property("g", &Color4::g)
      .add_property("b", &Color4::b)
      .add_property("a", &Color4::a)
      .def("set", &Color4::set)
      .def(py::self * GLfloat())
      .def(GLfloat() * py::self)
      .def(py::self *= GLfloat())
      .def("white", &Color4::white).staticmethod("white")
      .def("black", &Color4::black).staticmethod("black")
      .def("plexi", &Color4::plexi).staticmethod("plexi")
      .def("__repr__", Color4_str)
      .def("__iter__", py::range(&Color4_begin, &Color4_end))
      ;

  // shapes (not all exported)
  //TODO put them in a submodule?
  //TODO btScale/btUnscale

  py::class_<btCollisionShape, SmartPtr<btCollisionShape>, boost::noncopyable>("Shape", py::no_init);
#define EXPORT_SHAPE_CLASS(n,T,B) \
  py::class_<T, py::bases<B>, SmartPtr<T>, boost::noncopyable>(#n, py::no_init)

  EXPORT_SHAPE_CLASS(ShSphere,    btSphereShape,    btCollisionShape).def(py::init<btScalar>());
  EXPORT_SHAPE_CLASS(ShCapsule,   btCapsuleShape,   btCollisionShape).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShCapsuleX,  btCapsuleShapeX,  btCapsuleShape  ).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShCapsuleZ,  btCapsuleShapeZ,  btCapsuleShape  ).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShCone,      btConeShape,      btCollisionShape).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShConeX,     btConeShapeX,     btConeShape     ).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShConeZ,     btConeShapeZ,     btConeShape     ).def(py::init<btScalar, btScalar>());
  EXPORT_SHAPE_CLASS(ShCylinder,  btCylinderShape,  btCollisionShape).def(py::init<btVector3>());
  EXPORT_SHAPE_CLASS(ShCylinderX, btCylinderShapeX, btCylinderShape ).def(py::init<btVector3>());
  EXPORT_SHAPE_CLASS(ShCylinderZ, btCylinderShapeZ, btCylinderShape ).def(py::init<btVector3>());
  EXPORT_SHAPE_CLASS(ShBox,       btBoxShape,       btCollisionShape).def(py::init<btVector3>());
  // compound shape: special class (memory handling) and special constructor
  EXPORT_SHAPE_CLASS(ShCompound, CompoundShapeSmart, btCollisionShape)
      .def("__init__", py::make_constructor(&CompoundShape_init))
      ;

#undef EXPORT_SHAPE_CLASS

}


