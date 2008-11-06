#ifndef SENSOR_H
#define SENSOR_H

///@file

#include <ode/ode.h>
#include "global.h"


/** @brief Sensor
 *
 * Sensors are geoms which collide with object (but not other sensors).
 */
class Sensor
{
public:
  Sensor(dGeomID geom);

  /** @brief Test sensor collision and update resulting values
   *
   * Typically, function body will be like this:
   * \code
   * dSpaceCollide2(this->geom, (dGeomID)physics->get_space(), this, callback);
   * \endcode
   */
  virtual void update() = 0;

protected:
  dGeomID geom;

};


/// Ray sensor which returns object distance
class SensorRay: public Sensor
{
public:
  SensorRay(dGeomID geom);

  dReal get_dist() { return this->dist; }

  virtual void update()
  {
    dist = -1;
    dSpaceCollide2(this->geom, (dGeomID)physics->get_space(), this, ray_callback);
  }

  static void ray_callback(void *data, dGeomID o1, dGeomID o2);

private:
  /// Object distance (-1 if ray does not collide)
  dReal dist;
};



#endif
