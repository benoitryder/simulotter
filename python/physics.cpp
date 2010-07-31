#include "python/common.h"
#include "physics.h"


static btScalar Physics_get_earth_gravity() { return btUnscale(Physics::earth_gravity); }
static void Physics_set_earth_gravity(btScalar v) { Physics::earth_gravity = btScale(v); }
static btScalar Physics_get_margin_epsilon() { return btUnscale(Physics::margin_epsilon); }
static void Physics_set_margin_epsilon(btScalar v) { Physics::margin_epsilon = btScale(v); }
static btVector3 Physics_get_world_aabb_min() { return btUnscale(Physics::world_aabb_min); }
static void Physics_set_world_aabb_min(const btVector3 &v) { Physics::world_aabb_min = btScale(v); }
static btVector3 Physics_get_world_aabb_max() { return btUnscale(Physics::world_aabb_max); }
static void Physics_set_world_aabb_max(const btVector3 &v) { Physics::world_aabb_max = btScale(v); }


void python_export_physics()
{
  py::class_<Physics, SmartPtr<Physics>, boost::noncopyable>("Physics")
      .def("step", &Physics::step)
      .add_property("step_dt", &Physics::getStepDt)
      .add_property("time", &Physics::getTime)
      // statics
      .add_static_property("earth_gravity", &Physics_get_earth_gravity, &Physics_set_earth_gravity)
      .add_static_property("margin_epsilon", &Physics_get_margin_epsilon, &Physics_set_margin_epsilon)
      .add_static_property("world_aabb_min", &Physics_get_world_aabb_min, &Physics_set_world_aabb_min)
      .add_static_property("world_aabb_max", &Physics_get_world_aabb_max, &Physics_set_world_aabb_max)
      .def_readwrite("world_objects_max", &Physics::world_objects_max)
      ;
}


