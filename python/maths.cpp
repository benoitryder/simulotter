#include "python/common.h"


static const btScalar *vec2_begin(const btVector2 &v) { return v.xy; }
static const btScalar *vec2_end(const btVector2 &v) { return v.xy+2; }
static std::string vec2_str(const btVector2 &v)
{
  return stringf("<xy: %.2f %.2f>", v.x(), v.y());
}

static const btScalar *vec3_begin(const btVector3 &v) { return v.m_floats; }
static const btScalar *vec3_end(const btVector3 &v) { return v.m_floats+3; }
static std::string vec3_str(const btVector3 &v)
{
  return stringf("<xyz: %.2f %.2f %.2f>", v.x(), v.y(), v.z());
}


static btQuaternion *quat_init_matrix3(const btMatrix3x3 &m)
{
  btQuaternion *q = new btQuaternion();
  m.getRotation(*q);
  return q;
}
static btScalar quat_x(const btQuaternion &q) { return q.x(); }
static btScalar quat_y(const btQuaternion &q) { return q.y(); }
static btScalar quat_z(const btQuaternion &q) { return q.z(); }
static btScalar quat_w(const btQuaternion &q) { return q.w(); }
static const btScalar *quat_begin(const btQuaternion &q) { return &q[0]; }
static const btScalar *quat_end(const btQuaternion &q) { return &q[0]+4; }
static std::string quat_str(const btQuaternion &q)
{
  return stringf("<xyzw: %.2f %.2f %.2f %.2f>", q.x(), q.y(), q.z(), q.w());
}


static btMatrix3x3 *matrix3_init_def()
{
  btMatrix3x3 *m = new btMatrix3x3();
  m->setIdentity();
  return m;
}
static btMatrix3x3 *matrix3_init_rows(const btVector3 &r0, const btVector3 &r1, const btVector3 &r2)
{
  btMatrix3x3 *m = new btMatrix3x3();
  (*m)[0] = r0; (*m)[1] = r1; (*m)[2] = r2;
  return m;
}
static btMatrix3x3 *matrix3_init_euler(const btScalar yaw, const btScalar pitch, const btScalar roll)
{
  btMatrix3x3 *m = new btMatrix3x3();
#ifndef BT_EULER_DEFAULT_ZYX
  m->setEulerYPR(yaw, pitch, roll);
#else
  m->setEulerZYX(yaw, pitch, roll);
#endif
  return m;
}
static btVector3 matrix3_row(const btMatrix3x3 &m, int i)
{
  if( i < 0 || i > 2 ) {
    PyErr_SetObject(PyExc_IndexError, PyInt_FromLong(i));
    throw py::error_already_set(); 
  }
  return m.getRow(i);
}
static btVector3 matrix3_col(const btMatrix3x3 &m, int i)
{
  if( i < 0 || i > 2 ) {
    PyErr_SetObject(PyExc_IndexError, PyInt_FromLong(i));
    throw py::error_already_set(); 
  }
  return m.getColumn(i);
}
static py::tuple matrix3_get_euler_ypr(const btMatrix3x3 &m)
{
  btScalar y,p,r;
  m.getEulerYPR(y,p,r);
  return py::make_tuple(y,p,r);
}
static py::tuple matrix3_get_euler_zyx(const btMatrix3x3 &m)
{
  //XXX we don't support the extra solution_number parameter
  btScalar z,y,x;
  m.getEulerZYX(z,y,x);
  return py::make_tuple(z,y,x);
}
static const btVector3 *matrix3_begin(const btMatrix3x3 &m) { return &m[0]; }
static const btVector3 *matrix3_end(const btMatrix3x3 &m) { return &m[0]+3; }
static std::string matrix3_str(const btMatrix3x3 &m)
{
  return stringf("<m3x3: %.2f %.2f %.2f | %.2f %.2f %.2f | %.2f %.2f %.2f>",
                 m[0][0], m[0][1], m[0][2],
                 m[1][0], m[1][1], m[1][2],
                 m[2][0], m[2][1], m[2][2]
                );
}

// inspired by py::converter::implicit
struct matrix3_to_quat_converter
{
  static void* convertible(PyObject* obj)
  {
    return py::converter::implicit_rvalue_convertible_from_python(obj, py::converter::registered<btMatrix3x3>::converters)
        ? obj : 0;
  }

  static void construct(PyObject* obj, py::converter::rvalue_from_python_stage1_data* data)
  {
    void* storage = ((py::converter::rvalue_from_python_storage<btQuaternion>*)data)->storage.bytes;

    py::arg_from_python<btMatrix3x3> get_source(obj);
    bool convertible = get_source.convertible();
    BOOST_VERIFY(convertible);

    new (storage) btQuaternion();
    get_source().getRotation(*(btQuaternion *)storage);

    // record successful construction
    data->convertible = storage;
  }
};


static btTransform *trans_init_vec(const btVector3 &v)
{
  return new btTransform(btMatrix3x3::getIdentity(), v);
}
static std::string trans_str(const btTransform &t)
{
  const btVector3 &v = t.getOrigin();
  const btQuaternion q = t.getRotation();
  return stringf("<xyz: %.2f %.2f %.2f | xyzw: %.2f %.2f %.2f %.2f>",
                 v.x(), v.y(), v.z(), q.x(), q.y(), q.z(), q.w());
}


void python_export_maths()
{
  py::class_<btVector2>("vec2", py::no_init)
      .def(py::init<btScalar,btScalar>(
              (py::arg("x")=0, py::arg("y")=0)))
      .def(py::init<btVector2>())
      .add_property("x", py::make_function(&btVector2::x, py::return_value_policy<py::copy_const_reference>()))
      .add_property("y", py::make_function(&btVector2::y, py::return_value_policy<py::copy_const_reference>()))
      .def("dot", &btVector2::dot)
      .def("length2", &btVector2::length2)
      .def("length", &btVector2::length)
      .def("distance2", &btVector2::distance)
      .def("distance", &btVector2::distance)
      .def("normalize", &btVector2::normalized)
      .def("rotate", &btVector2::rotated)
      .def("angle", &btVector2::angle)
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def( -py::self )
      .def(py::self * btScalar())
      .def(py::self * py::self)
      .def(btScalar() * py::self)
      .def(py::self / btScalar())
      .def("__abs__", &btVector2::absolute)
      .def("__repr__", vec2_str)
      .def("__iter__", py::range(&vec2_begin, &vec2_end))
      ;

  py::class_<btVector3>("vec3", py::no_init)
      .def(py::init<btScalar,btScalar,btScalar>(
              (py::arg("x")=0, py::arg("y")=0, py::arg("z")=0)))
      .def(py::init<btVector3>())
      .add_property("x", py::make_function(&btVector3::x, py::return_value_policy<py::copy_const_reference>()))
      .add_property("y", py::make_function(&btVector3::y, py::return_value_policy<py::copy_const_reference>()))
      .add_property("z", py::make_function(&btVector3::z, py::return_value_policy<py::copy_const_reference>()))
      .def("dot", &btVector3::dot)
      .def("length2", &btVector3::length2)
      .def("length", &btVector3::length)
      .def("distance2", &btVector3::distance)
      .def("distance", &btVector3::distance)
      .def("normalize", &btVector3::normalized)
      .def("rotate", &btVector3::rotate)
      .def("angle", &btVector3::angle)
      .def("cross", &btVector3::cross)
      .def("triple", &btVector3::triple)
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def( -py::self )
      .def(py::self * btScalar())
      .def(py::self * py::self)
      .def(btScalar() * py::self)
      .def(py::self / btScalar())
      .def("__abs__", &btVector3::absolute)
      .def("__repr__", vec3_str)
      .def("__iter__", py::range(&vec3_begin, &vec3_end))
      ;

  py::implicitly_convertible<btVector2,btVector3>();
  py::implicitly_convertible<btVector3,btVector2>();


  // ambiguous unary minus operator, cannot use .def( - (py::self) )
  btQuaternion (btQuaternion::*quat_unary_neg)() const = &btQuaternion::operator-;

  py::class_<btQuaternion>("quat", py::no_init)
      .def(py::init<btScalar,btScalar,btScalar,btScalar>(
              (py::arg("x")=0, py::arg("y")=0, py::arg("z")=0, py::arg("w")=1)))
      .def(py::init<btVector3,btScalar>(
              (py::arg("axis"), py::arg("angle")=0)))
      .def(py::init<btScalar,btScalar,btScalar>(
              (py::arg("yaw")=0, py::arg("pitch")=0, py::arg("roll")=0)))
      .def("__init__", py::make_constructor(&quat_init_matrix3))
      // using py::make_function() does not work because accessors are defined
      // on btQuadWord and not directly on btQuaternion
      .add_property("x", quat_x)
      .add_property("y", quat_y)
      .add_property("z", quat_z)
      .add_property("w", quat_w)
      .def("dot", &btQuaternion::dot)
      .def("length2", &btQuaternion::length2)
      .def("length", &btQuaternion::length)
      .def("normalize", &btQuaternion::normalized)
      .def("angle_with", &btQuaternion::angle)
      .add_property("angle", &btQuaternion::getAngle)
      .add_property("axis", &btQuaternion::getAxis)
      .def("inverse", &btQuaternion::inverse)
      .def("farthest", &btQuaternion::farthest)
      .def("nearest", &btQuaternion::nearest)
      .def("rotate", &quatRotate)
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def("__neg__", quat_unary_neg)
      .def(py::self * btScalar())
      .def(py::self * btVector3())
      .def(btVector3() * py::self)
      .def(py::self * py::self)
      .def(py::self / btScalar())
      .def("__repr__", quat_str)
      .def("__iter__", py::range(&quat_begin, &quat_end))
      ;

  py::class_<btMatrix3x3>("matrix3", py::no_init)
      .def("__init__", py::make_constructor(&matrix3_init_def))
      .def("__init__", py::make_constructor(&matrix3_init_rows))
      .def(py::init< btScalar,btScalar,btScalar,
           btScalar,btScalar,btScalar,
           btScalar,btScalar,btScalar >())
      .def("__init__", py::make_constructor(
              &matrix3_init_euler, py::default_call_policies(), (
                  py::arg("yaw")=0, py::arg("pitch")=0, py::arg("roll")=0)))
      .def(py::init<btMatrix3x3>())
      .def("row", &matrix3_row)
      .def("col", &matrix3_col)
      .def("__getitem__", &matrix3_row)
      .add_property("euler_ypr", &matrix3_get_euler_ypr)
      .add_property("euler_zyx", &matrix3_get_euler_zyx)
      .def("scale", &btMatrix3x3::scaled)
      .def("determinant", &btMatrix3x3::determinant)
      .def("adjoint", &btMatrix3x3::adjoint)
      .def("transpose", &btMatrix3x3::transpose)
      .def("transpose_times", &btMatrix3x3::transposeTimes)
      .def("times_transpose", &btMatrix3x3::timesTranspose)
      .def("inverse", &btMatrix3x3::inverse)
      .def("tdotx", &btMatrix3x3::tdotx)
      .def("tdoty", &btMatrix3x3::tdoty)
      .def("tdotz", &btMatrix3x3::tdotz)
      .def("cofac", &btMatrix3x3::cofac)
      .def(py::self == py::self)
      .def(py::self * py::self)
      .def(py::self * btVector3())
      .def(btVector3() * py::self)
      .def("__abs__", &btMatrix3x3::absolute)
      .def("__repr__", matrix3_str)
      .def("__iter__", py::range(&matrix3_begin, &matrix3_end))
      ;

  py::implicitly_convertible<btQuaternion,btMatrix3x3>();
  // implicit conversion from matrix3 to quat
  // inspired by py::implicitly_convertible
  py::converter::registry::push_back(
      &matrix3_to_quat_converter::convertible
      , &matrix3_to_quat_converter::construct
      , py::type_id<btQuaternion>()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
      , &py::converter::expected_from_python_type_direct<btMatrix3x3>::get_pytype
#endif
      );

  // explicit some function pointers to help the compiler
  const btMatrix3x3 &(btTransform::*const trans_get_basis)() const = &btTransform::getBasis;
  const btVector3 &(btTransform::*const trans_get_origin)() const = &btTransform::getOrigin;

  py::class_<btTransform>("trans", py::no_init)
      .def(py::init<btQuaternion, btVector3>(
              (py::arg("rot"), py::arg("origin")=btVector3(0,0,0))))
      .def(py::init<btMatrix3x3, btVector3>(
              (py::arg("basis")=btMatrix3x3::getIdentity(),
               py::arg("origin")=btVector3(0,0,0))))
      .def("__init__", py::make_constructor(&trans_init_vec))
      .def(py::init<btTransform>())
      .add_property("basis", py::make_function(trans_get_basis, py::return_value_policy<py::return_by_value>()))
      .add_property("origin", py::make_function(trans_get_origin, py::return_value_policy<py::return_by_value>()))
      .add_property("rotation", &btTransform::getRotation)
      .def("inverse", &btTransform::inverse)
      .def("inverse_times", &btTransform::inverseTimes)
      .def(py::self == py::self)
      .def(py::self * py::self)
      .def(py::self * btVector3())
      .def(py::self * btQuaternion())
      .def("__repr__", trans_str)
      ;
}


