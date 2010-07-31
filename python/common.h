#ifndef PYTHON_COMMON_H_
#define PYTHON_COMMON_H_

/** @file
 * @brief Common declarations for Python binding.
 */

// this header is precompiled and thus include some "extra" headers.
#include <boost/python.hpp>
#include "smart.h"
#include "maths.h"
#include "log.h"

namespace py = boost::python;


/// Allow to use SmartPtr as Holder for Python classes.
template <class T> T* get_pointer(SmartPtr<T> const& p) { return p.get(); }


#define SIMULOTTER_MODULE_NAME simulotter

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)
#define SIMULOTTER_MODULE_NAME_STR QUOTE(SIMULOTTER_MODULE_NAME)


/** @brief Create a submodule.
 *
 * The submodule is created in the current Python scope. New items will be
 * added to the submodule's scope until the end of the current C++ scope.
 *
 * This macro cannot be called several times in the same C++ scope.
 */
#define SIMULOTTER_PYTHON_SUBMODULE(name) \
  py::object submodule_(py::handle<>(py::borrowed(PyImport_AddModule( \
      SIMULOTTER_MODULE_NAME_STR "." #name )))); \
  py::scope().attr(#name) = submodule_; \
  py::scope subscope_ = submodule_;


#endif
