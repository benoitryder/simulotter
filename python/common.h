#ifndef PYTHON_COMMON_H_
#define PYTHON_COMMON_H_

/** @file
 * @brief Common declarations for Python binding
 */

#ifdef _WIN32
// workaround for pyconfig.h defining hypot to _hypot (which is bad)
#include <cmath>
#endif
#ifdef _WIN64
// workaround for python bug #4709
#define MS_WIN64
#endif
#include <boost/python.hpp>
#include "smart.h"
#include "maths.h"

namespace py = boost::python;


/** @name SmartPtr support
 *
 * Code is mainly inspired by shared_ptr special handling in Boost.Python.
 */
//@{

/// Allow to use SmartPtr as Holder for Python classes
template <class T> T* get_pointer(const SmartPtr<T>& p) { return p.get(); }


namespace boost { namespace python { namespace converter {

template <class T>
struct SmartPtr_from_python
{
  SmartPtr_from_python()
  {
    converter::registry::insert(&convertible, &construct, type_id<SmartPtr<T>>()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
                                , &converter::expected_from_python_type_direct<T>::get_pytype
#endif
                               );
  }

 private:
  static void* convertible(PyObject* p)
  {
    if(p == Py_None) {
      return p;
    }
    return converter::get_lvalue_from_python(p, registered<T>::converters);
  }

  static void construct(PyObject* source, rvalue_from_python_stage1_data* data)
  {
    void* const storage = ((converter::rvalue_from_python_storage<SmartPtr<T>>*)data)->storage.bytes;
    // Deal with the "None" case.
    if(data->convertible == source) {
      new (storage) SmartPtr<T>();
    } else {
      new (storage) SmartPtr<T>(static_cast<T*>(data->convertible));
    }

    data->convertible = storage;
  }
};

}}}


/// Register SmartPtr<T> conversions
template <class T>
void py_smart_register()
{
  py::converter::SmartPtr_from_python<T>();
}


//@}


#define SIMULOTTER_MODULE_NAME _simulotter

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)
#define SIMULOTTER_MODULE_NAME_STR QUOTE(SIMULOTTER_MODULE_NAME)


/** @brief Create a submodule
 *
 * The submodule is created in the current Python scope. New items will be
 * added to the submodule's scope until the end of the current C++ scope.
 *
 * This macro cannot be called several times in the same C++ scope.
 */
#define SIMULOTTER_PYTHON_SUBMODULE(name) \
  py::object submodule_(py::handle<>(py::borrowed(PyImport_AddModule( \
      (py::extract<std::string>(py::scope().attr("__name__"))() \
       + "." #name).c_str())))); \
  py::scope().attr(#name) = submodule_; \
  py::scope subscope_ = submodule_;


#endif
