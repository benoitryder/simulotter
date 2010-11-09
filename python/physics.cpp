#include "python/common.h"
#include <boost/python/stl_iterator.hpp>
#include "physics.h"


static btScalar Physics_get_world_gravity() { return btUnscale(Physics::world_gravity); }
static void Physics_set_world_gravity(btScalar v) { Physics::world_gravity = btScale(v); }
static btScalar Physics_get_margin_epsilon() { return btUnscale(Physics::margin_epsilon); }
static void Physics_set_margin_epsilon(btScalar v) { Physics::margin_epsilon = btScale(v); }
static btVector3 Physics_get_world_aabb_min() { return btUnscale(Physics::world_aabb_min); }
static void Physics_set_world_aabb_min(const btVector3 &v) { Physics::world_aabb_min = btScale(v); }
static btVector3 Physics_get_world_aabb_max() { return btUnscale(Physics::world_aabb_max); }
static void Physics_set_world_aabb_max(const btVector3 &v) { Physics::world_aabb_max = btScale(v); }

static void Physics_task_cb(py::object cb, Physics *ph)
{
  py::call<void>(cb.ptr(), py::ptr(ph));
}

static void Physics_task_iter(SmartPtr<TaskBasic> task, py::object iter, Physics *)
{
  // note: instantiating the iterator iterate it
  py::stl_input_iterator<void*> it(iter), end;
  if( it == end ) {
    task->cancel(); // end, do nothing
  }
}

static SmartPtr<TaskBasic> Task_init(py::object cb, py::object period)
{
  SmartPtr<TaskBasic> task;

  if( period.ptr() == Py_None ) {
    task = new TaskBasic();
  } else {
    task = new TaskBasic( py::extract<btScalar>(period) );
  }

  if( PyObject_HasAttrString(cb.ptr(), "__iter__") ) {
    task->setCallback( boost::bind(&Physics_task_iter, task, cb, _1) );
  } else if( !PyCallable_Check(cb.ptr()) ) {
    PyErr_SetString(PyExc_TypeError, "callback is not callable");
    throw py::error_already_set(); 
  } else {
    task->setCallback( boost::bind(&Physics_task_cb, cb, _1) );
  }
  return task;
}

static SmartPtr<TaskBasic> Physics_schedule_task(Physics &ph, TaskBasic *task, py::object time)
{
  btScalar cpp_time = -1;
  if( time.ptr() != Py_None ) {
    cpp_time = py::extract<btScalar>(time);
  }
  ph.scheduleTask(task, cpp_time);
  return task;
}

static SmartPtr<TaskBasic> Physics_schedule_cb(Physics &ph, py::object cb, py::object period, py::object time)
{
  SmartPtr<TaskBasic> cpp_task = Task_init(cb, period);
  return Physics_schedule_task(ph, cpp_task, time);
}

void python_export_physics()
{
  py::scope in_Physics = py::class_<Physics, SmartPtr<Physics>, boost::noncopyable>("Physics", py::no_init)
      .def(py::init<btScalar>((py::arg("step_dt")=0.002)))
      .def("step", &Physics::step)
      .add_property("step_dt", &Physics::getStepDt)
      .add_property("time", &Physics::getTime)
      .def("schedule", &Physics_schedule_task, ( py::arg("task"), py::arg("time")=py::object() ))
      .def("schedule", &Physics_schedule_cb, ( py::arg("cb"), py::arg("period"), py::arg("time")=py::object() ))
      // statics
      .add_static_property("world_gravity", &Physics_get_world_gravity, &Physics_set_world_gravity)
      .add_static_property("margin_epsilon", &Physics_get_margin_epsilon, &Physics_set_margin_epsilon)
      .add_static_property("world_aabb_min", &Physics_get_world_aabb_min, &Physics_set_world_aabb_min)
      .add_static_property("world_aabb_max", &Physics_get_world_aabb_max, &Physics_set_world_aabb_max)
      .def_readwrite("world_objects_max", &Physics::world_objects_max)
      ;

  py::class_<TaskBasic, SmartPtr<TaskBasic>, boost::noncopyable>("Task", py::no_init)
      .def("__init__", py::make_constructor(&Task_init, py::default_call_policies(), (
                  py::arg("cb"), py::arg("period")=py::object() )))
      .def("cancel", &TaskBasic::cancel)
      .add_property("cancelled", &TaskBasic::cancelled)
      ;
}

