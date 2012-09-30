#include "python/common.h"
#include "sensors.h"


static SmartPtr<SRay> SRay_init(btScalar min, btScalar max) { return new SRay(btScale(min), btScale(max)); }
static py::object SRay_hitTest(const SRay& o)
{
  btScalar d = o.hitTest();
  if(d < 0) {
    return py::object();
  }
  return py::object(btUnscale(d));
}
static SmartPtr<Object> SRay_get_attach_obj(const SRay& o) { return o.getAttachObject(); }
static void SRay_set_attach_obj(SRay& o, const SmartPtr<Object>& obj) { o.setAttachObject(obj); }
static btTransform SRay_get_attach_point(const SRay& o) { return btUnscale(o.getAttachPoint()); }
static void SRay_set_attach_point(SRay& o, const btTransform& tr) { o.setAttachPoint(btScale(tr)); }

void python_export_sensors()
{
  py::class_<SRay, py::bases<Object>, SmartPtr<SRay>, boost::noncopyable>("SRay", py::no_init)
      .def("__init__", py::make_constructor(&SRay_init))
      .def("hitTest", &SRay_hitTest)
      .add_property("attach_obj", &SRay_get_attach_obj, &SRay_set_attach_obj)
      .add_property("attach_point", &SRay_get_attach_point, &SRay_set_attach_point)
      .add_property("color", &SRay::getColor, &SRay::setColor)
      ;
}

