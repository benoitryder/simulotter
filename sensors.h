#ifndef SENSORS_H
#define SENSORS_H

///@file

#include "object.h"


/** @brief Ray sensor
 *
 * A sensor is not a physical object and is not attached to a given object.
 * This class provides methods to test a sensor hit.
 * The sensor is oriented along X axis.
 */
class SRay: public Object
{
public:
  SRay(btScalar min, btScalar max);
  virtual ~SRay();

  virtual const btTransform &getTrans() const { return trans_; }
  virtual void setTrans(const btTransform &tr) { trans_ = tr; }

  /// Draw the sensor hit zone
  virtual void draw(Display *d);

  /** @brief Get hit distance
   * @retval a positive value in sensor range on success, -1.0 otherwise
   */
  btScalar hitTest() const;

protected:
  btTransform trans_;
  /// Hit range
  btScalar range_min_, range_max_;

public:

  /// GP2D12 sensor
  static const SRay gp2d12;
};


#endif
