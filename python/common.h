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


/// Allow to use SmartPtr as Holder for Python classes.
template <class T> T* get_pointer(SmartPtr<T> const& p) { return p.get(); }

namespace py = boost::python;


#endif
