#ifndef EUROBOT2013_H_
#define EUROBOT2013_H_

/** @file
 * @brief Implementation of Eurobot 2013 rules, Happy Birthday!
 */

#include <memory>
#include "object.h"
#include "galipeur.h"

namespace eurobot2013 {


class OGround2013: public OGround
{
 public:
  static const btVector2 SIZE;
  static const btScalar SQUARE_SIZE;

  OGround2013();
  virtual ~OGround2013() {}

 protected:
  virtual void drawDisplayList() const;
};


/** @brief The cake
 *
 * The physical cake is built with three cylinders (for the stairs) and several
 * thin boxes at the top to simulate a hollow cylinder.
 * The dividing wall inside the basket is approximated to a box (no round
 * corner).
 *
 * The displayed cake will not reflect the physical one: half cylinders will be
 * drawn. The other half will collide but since it is outside the table it does
 * not matter.
 *
 * The cake object does not include candles.
 */
class OCake: public OSimple
{
  static constexpr unsigned int BASKET_SLICES = 11; // odd number is sligthly better
 public:
  static constexpr btScalar LEVEL_HEIGHT = 0.100_m; // per level increment
  static constexpr btScalar LEVEL_RADIUS = 0.100_m; // per level decrement
  static constexpr btScalar BASE_RADIUS = 0.500_m; // cake bottom radius
  static constexpr btScalar BASKET_HEIGHT = 0.200_m;
  static constexpr btScalar BASKET_RADIUS = 0.200_m;
  static constexpr btScalar BASKET_WIDTH = 0.003_m;
  static constexpr btScalar BASKET_WALL_HEIGHT = 0.250_m;
  static constexpr btScalar BACK_HEIGHT = 0.700_m;
  static constexpr btScalar BACK_WIDTH = 0.010_m;

  OCake();
  virtual void draw(Display* d) const;
  virtual void drawLast(Display* d) const;

 private:
  static SmartPtr<btCompoundShape> shape_;
  static std::vector<SmartPtr<btCylinderShapeZ>> shape_levels_;
  static btCompoundShape shape_basket_;
  static btBoxShape shape_basket_slice_;
  static btBoxShape shape_basket_wall_;
  static btBoxShape shape_back_;
};


/// Gift to be attached to a support
class OGift: public OSimple
{
 public:
  static const btVector3 SIZE;
  OGift();

 private:
  static SmartPtr<btBoxShape> shape_;
};


/** @brief Gift support
 *
 * Origin is located at the anchor point on the table.
 *
 * Gifts are created with the support and attached to it.
 */
class OGiftSupport: public OSimple
{
 public:
  static const btVector3 SIZE;
  OGiftSupport();

  virtual void setTrans(const btTransform& tr);
  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();
  virtual void draw(Display* d) const;

 private:
  static SmartPtr<btCompoundShape> shape_;
  static btBoxShape shape_x_;
  static btBoxShape shape_y_;

  void initGift(unsigned int n);
  void resetGiftTrans(unsigned int n);

  OGift gifts_[2];
  std::unique_ptr<btHingeConstraint> gift_links_[2];
};


/// Glass
class OGlass: public OSimple
{
  static constexpr unsigned int SLICES = 12;
 public:
  static constexpr btScalar HEIGHT = 0.080_m;
  static constexpr btScalar RADIUS = 0.080_m/2;
  static constexpr btScalar INNER_RADIUS = 0.074_m/2;
  static constexpr btScalar BOTTOM_HEIGHT = 0.005_m;
  OGlass();

  virtual void draw(Display* d) const;
  virtual void drawLast(Display* d) const;

 private:
  static SmartPtr<btCompoundShape> shape_;
  static btBoxShape shape_slice_;
  static btCylinderShapeZ shape_bottom_;
};


/// Candle flame (tennis ball)
class OCandleFlame: public OSimple
{
 public:
  static constexpr btScalar RADIUS = 0.065_m/2;
  OCandleFlame();

 private:
  static SmartPtr<btSphereShape> shape_;
};


/** @brief Candle
 *
 * Flame is created with the candle.
 */
class OCandle: public OSimple
{
 public:
  static constexpr btScalar HEIGHT = 0.050_m;
  static constexpr btScalar RADIUS = 0.080_m/2;
  OCandle();

  virtual void setTrans(const btTransform& tr);
  virtual void addToWorld(Physics* physics);
  virtual void removeFromWorld();
  virtual void draw(Display* d) const;

 private:
  static SmartPtr<btCylinderShapeZ> shape_;

  void resetFlameTrans();

  OCandleFlame flame_;
  std::unique_ptr<btSliderConstraint> flame_link_;
};


}

#endif
