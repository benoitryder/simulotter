
#include "sensor.h"
#include "maths.h"


Sensor::Sensor(dGeomID geom)
{
  this->geom = geom;
}


SensorRay::SensorRay(dGeomID geom): Sensor(geom)
{
  if( dGeomGetClass(geom) != dRayClass )
  {
    throw Error("SensorRay geom is not a ray");
  }
}

void SensorRay::ray_callback(void *data, dGeomID o1, dGeomID o2)
{
  dContactGeom c_geom; 

  if( dCollide(o1, o2, 1, &c_geom, sizeof(c_geom)) > 0 )
    ((SensorRay*)data)->dist = MIN(((SensorRay*)data)->dist, c_geom.depth);
}

