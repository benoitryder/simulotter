#ifndef SENSOR_H
#define SENSOR_H

#include "global.h"

///@file


/** @brief Ray sensor
 *
 * A sensor is not a physical object and is not attached to a given object.
 * This class provides methods to test a sensor hit and the same instance can
 * be used for several tests with different transformations.
 *
 * The sensor is oriented along X axis.
 */
class SRay: public SmartObject
{
public:
  SRay(btScalar min, btScalar max);
  virtual ~SRay();

  /// Draw the sensor hit zone
  virtual void draw() const;

  /** @brief Get hit distance
   *
   * @retval a positive value in sensor range on success, -1.0 otherwise
   */
  btScalar hitTest(const btTransform &origin) const;

protected:
  /// Hit range
  btScalar range_min, range_max;

public:

  /// GP2D12 sensor
  static const SRay gp2d12;
};


#endif
