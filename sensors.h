#ifndef SENSORS_H
#define SENSORS_H

///@file

#include "object.h"


/** @brief Ray sensor
 *
 * A sensor is not a physical object. It may be positioned in world coordinates
 * or relatively to an object (attached to it).
 * The getTrans() and setTrans() method are provided to satisfy Object
 * interface but should not be needed when attaching the sensor to an object
 * and getAttachPoint() and setAttachPoint() should be used instead.
 *
 * This class provides methods to test a sensor hit.
 * The sensor ray is oriented along X axis.
 */
class SRay: public Object
{
 public:
  SRay(btScalar min, btScalar max);
  virtual ~SRay();

  /** @brief Get hit distance
   * @retval a positive value in sensor range on success, -1.0 otherwise
   */
  btScalar hitTest() const;

  Object* getAttachObject() const { return obj_; }
  /** @brief Attach the sensor to an object
   *
   * The sensor is removed from its current world and added to the one of the
   * given object. If \e obj is \e NULL, the sensor is detached from its
   * current object but not removed from its world.
   */
  void setAttachObject(Object* obj);

  const btTransform& getAttachPoint() const { return attach_; }
  void setAttachPoint(const btTransform& tr) { attach_ = tr; }

  virtual void removeFromWorld();

  virtual const btTransform getTrans() const;
  virtual void setTrans(const btTransform& tr);

  /// Draw the sensor hit zone
  virtual void draw(Display* d) const;

  Color4 getColor() const { return color_; }
  void setColor(const Color4& color) { color_ = color; }

 protected:
  /// Attach point
  btTransform attach_;
  /// Reference object for attached sensors
  SmartPtr<Object> obj_;
  /// Hit range
  btScalar range_min_, range_max_;

  Color4 color_;
};


#endif
