#include <GL/freeglut.h>
#include "sensors.h"
#include "physics.h"
#include "log.h"


SRay::SRay(btScalar min, btScalar max):
  range_min_(min), range_max_(max)
{
  if( min < 0 || max <= min )
    throw(Error("invalid sensor range: %f - %d", min, max));
}

SRay::~SRay()
{
}


void SRay::draw() const
{
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  btglVertex3(range_min_,0,0);
  btglVertex3(range_max_,0,0);
  glEnd();
  glEnable(GL_LIGHTING);
}


btScalar SRay::hitTest(const btTransform &origin) const
{
  btVector3 ray_from = origin * btVector3(range_min_,0,0);
  btVector3 ray_to   = origin * btVector3(range_max_,0,0);
  btCollisionWorld::ClosestRayResultCallback ray_cb( ray_from, ray_to );

  Physics::physics->getWorld()->rayTest( ray_from, ray_to, ray_cb );

  if( ray_cb.hasHit() )
    return ray_cb.m_closestHitFraction + range_min_;
  else
    return -1.0;
}


const SRay SRay::gp2d12( btScale(0.10), btScale(0.80) );

