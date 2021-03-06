#include "display.h"
#include "sensors.h"
#include "physics.h"
#include "log.h"


SRay::SRay(btScalar min, btScalar max):
  range_min_(min), range_max_(max),
  color_(Color4::white)
{
  if(min < 0 || max <= min) {
    throw(Error("invalid sensor range: %f - %f", min, max));
  }
}

SRay::~SRay()
{
}


void SRay::setAttachObject(Object* obj)
{
  if(obj == NULL) {
    obj_ = NULL;
  } else {
    if(!obj->getPhysics()) {
      throw(Error("attached object is not in a world"));
    }
    if(obj_ != NULL) {
      removeFromWorld();
    }
    addToWorld(obj->getPhysics());
    obj_ = obj;
  }
}


void SRay::removeFromWorld()
{
  obj_ = NULL;
  Object::removeFromWorld();
}

const btTransform SRay::getTrans() const
{
  if(!obj_) {
    return attach_;
  } else {
    return obj_->getTrans() * attach_;
  }
}

void SRay::setTrans(const btTransform& tr)
{
  if(!obj_) {
    attach_ = tr;
  } else {
    attach_ = obj_->getTrans().inverse() * tr;
  }
}

btScalar SRay::hitTest() const
{
  const btTransform tr = getTrans();
  btVector3 ray_from = tr * btVector3(range_min_, 0, 0);
  btVector3 ray_to = tr * btVector3(range_max_, 0, 0);
  btCollisionWorld::ClosestRayResultCallback ray_cb( ray_from, ray_to );

  physics_->getWorld()->rayTest( ray_from, ray_to, ray_cb );

  if(ray_cb.hasHit()) {
    return ray_cb.m_closestHitFraction + range_min_;
  } else {
    return -1.0;
  }
}


void SRay::draw(Display*) const
{
  glPushMatrix();
  drawTransform(getTrans());
  glColor4fv(color_);

  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  btglVertex3(range_min_, 0, 0);
  btglVertex3(range_max_, 0, 0);
  glEnd();
  glEnable(GL_LIGHTING);

  glPopMatrix();
}


