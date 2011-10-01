#include "display.h"
#include "physics.h"
#include "object.h"
#include "log.h"


void Object::drawTransform(const btTransform &transform)
{
  btScalar m[16];
  transform.getOpenGLMatrix(m);
  btglMultMatrix(m);
}

void Object::drawShape(const btCollisionShape *shape)
{
  glPushMatrix();
  switch( shape->getShapeType() )
  {
    case COMPOUND_SHAPE_PROXYTYPE:
      {
        const btCompoundShape *compound_shape = static_cast<const btCompoundShape*>(shape);
        for( int i=compound_shape->getNumChildShapes()-1; i>=0; i-- )
        {
          glPushMatrix();
          drawTransform(compound_shape->getChildTransform(i));
          drawShape(compound_shape->getChildShape(i));
          glPopMatrix();
        }
        break;
      }
    case SPHERE_SHAPE_PROXYTYPE:
      {
        const btSphereShape *sphere_shape = static_cast<const btSphereShape*>(shape);
        glutSolidSphere(sphere_shape->getRadius(), Display::draw_div, Display::draw_div);
        break;
      }
    case BOX_SHAPE_PROXYTYPE:
      {
        const btBoxShape *box_shape = static_cast<const btBoxShape*>(shape);
        const btVector3 &size = box_shape->getHalfExtentsWithMargin();
        btglScale(2*size[0], 2*size[1], 2*size[2]);
        glutSolidCube(1.0);
        break;
      }
    case CAPSULE_SHAPE_PROXYTYPE:
      {
        const btCapsuleShape *capsule_shape = static_cast<const btCapsuleShape*>(shape);
        switch( capsule_shape->getUpAxis() )
        {
          case 0: btglRotate(-90.0, 0.0, 1.0, 0.0); break;
          case 1: btglRotate(-90.0, 1.0, 0.0, 0.0); break;
          case 2: break;
          default:
            throw(Error("invalid capsule up axis"));
        }
        const btScalar r = capsule_shape->getRadius();
        const btScalar len = capsule_shape->getHalfHeight();
        btglTranslate(0, 0, -len);
        glutSolidCylinder(r, 2*len, Display::draw_div, Display::draw_div);
        glutSolidSphere(r, Display::draw_div, Display::draw_div);
        btglTranslate(0, 0, 2*len);
        glutSolidSphere(r, Display::draw_div, Display::draw_div);
        break;
      }
    case CYLINDER_SHAPE_PROXYTYPE:
      {
        const btCylinderShape *cylinder_shape = static_cast<const btCylinderShape*>(shape);
        const int axis = cylinder_shape->getUpAxis();
        const btScalar r = cylinder_shape->getRadius();
        // there is not a getHalfHeight() function
        const btVector3 &size = cylinder_shape->getHalfExtentsWithMargin();
        const btScalar len = size[axis];
        switch( axis )
        {
          case 0: btglRotate(-90.0, 0.0, 1.0, 0.0); break;
          case 1: btglRotate(-90.0, 1.0, 0.0, 0.0); break;
          case 2: break;
          default:
            throw(Error("invalid cylinder up axis"));
        }
        btglTranslate(0, 0, -len);
        glutSolidCylinder(r, 2*len, Display::draw_div, Display::draw_div);
        break;
      }
    case CONE_SHAPE_PROXYTYPE:
      {
        const btConeShape *cone_shape = static_cast<const btConeShape*>(shape);
        const int axis = cone_shape->getConeUpIndex();
        const btScalar r = cone_shape->getRadius();
        const btScalar h = cone_shape->getHeight();
        switch( axis )
        {
          case 0: btglRotate(-90.0, 0.0, 1.0, 0.0); break;
          case 1: btglRotate(-90.0, 1.0, 0.0, 0.0); break;
          case 2: break;
          default:
            throw(Error("invalid cone up axis"));
        }
        btglTranslate(0, 0, -h/2);
        glutSolidCone(r, h, Display::draw_div, Display::draw_div);
        break;
      }
    default:
      throw(Error("drawing not supported for this geometry class"));
      break;
  }
  glPopMatrix();
}

void Object::addToWorld(Physics *physics)
{
  assert( physics != NULL );
  if( physics_ != NULL ) {
    throw(Error("object is already in a world"));
  }
  if( physics->getObjs().insert(this).second == false ) {
    throw(Error("object added to the world twice"));
  }
  physics_ = physics;
}

void Object::removeFromWorld()
{
  if( physics_ == NULL )
    throw(Error("object is not in a world"));
  this->disableTickCallback();
  physics_->getObjs().erase(this); // should return 1
  physics_ = NULL;
}

void Object::tickCallback()
{
  throw(Error("non-implemented tickCallback() called"));
}

void Object::enableTickCallback()
{
  if( physics_ == NULL )
    throw(Error("object is not in a world"));
  physics_->getTickObjs().insert(this);
}

void Object::disableTickCallback()
{
  if( physics_ == NULL )
    throw(Error("object is not in a world"));
  physics_->getTickObjs().erase(this);
}


OSimple::OSimple():
    btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL)),
    color_(Color4())
{
}

OSimple::OSimple(btCollisionShape *shape, btScalar mass):
    btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL)),
    color_(Color4())
{
  setShape(shape);
  if( mass ) {
    setMass(mass);
  }
}

OSimple::~OSimple()
{
  // release shape held by Bullet
  btCollisionShape *sh = getCollisionShape();
  if( sh != NULL )
    SmartPtr_release(sh);
}

void OSimple::setShape(btCollisionShape *shape)
{
  if( getCollisionShape() != NULL ) {
    throw(Error("cannot reassign shape"));
  }
  if( shape == NULL ) {
    throw(Error("invalid shape"));
  }
  setCollisionShape(shape);
  // count shape reference held by Bullet
  SmartPtr_add_ref(shape);
}

void OSimple::setMass(btScalar mass)
{
  if( getCollisionShape() == NULL )
    throw(Error("object shape must be set to set its mass"));

  btVector3 inertia(0,0,0);
  if( mass )
    getCollisionShape()->calculateLocalInertia(mass, inertia);

  setMassProps(mass, inertia);
  updateInertiaTensor();
}

void OSimple::addToWorld(Physics *physics)
{
  if( !isInitialized() )
    throw(Error("object must be initialized to be added to a world"));
  physics->getWorld()->addRigidBody(this);
  Object::addToWorld(physics);
}

void OSimple::removeFromWorld()
{
  Physics *ph_bak = physics_;
  Object::removeFromWorld();
  ph_bak->getWorld()->removeRigidBody(this);
}


void OSimple::setPosAbove(const btVector2 &pos)
{
  btVector3 aabbMin, aabbMax;
  this->getAabb(aabbMin, aabbMax);
  setPos( btVector3(pos.x(), pos.y(), -aabbMin.z() + Physics::margin_epsilon) );
}


void OSimple::draw(Display *d) const
{
  if( color_.a() >= 0.95 ) {
    this->drawObject(d);
  }
}

void OSimple::drawLast(Display *d) const
{
  if( color_.a() < 0.95 ) {
    this->drawObject(d);
  }
}

void OSimple::drawObject(Display *d) const
{
  glColor4fv(color_);
  glPushMatrix();
  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(m_collisionShape) ) {
    drawShape(m_collisionShape);
    d->endDisplayList();
  }

  glPopMatrix();
}


OGround::OGround(const btVector2 &size, const Color4 &color, const Color4 &color_t1, const Color4 &color_t2):
    size_(btVector3(size.x(), size.y(), btScale(0.1))),
    start_size_(btScale(0.5)),
    shape_(new btBoxShape(size_/2))
{
  setShape(shape_);
  setPos( btVector3(0, 0, -size_[2]/2) );
  setColor(color);
  color_t1_ = color_t1;
  color_t2_ = color_t2;
}

OGround::~OGround()
{
}


void OGround::draw(Display *d) const
{
  glPushMatrix();

  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(this) ) {
    // Their should be only one ground instance, thus we create one display
    // list per instance. This allow to put color changes in it.
    drawBase();
    drawStartingAreas();
    d->endDisplayList();
  }

  glPopMatrix();
}

void OGround::drawBase() const
{
  glPushMatrix();

  glColor4fv(color_);
  btglScale(size_[0], size_[1], size_[2]);
  glutSolidCube(1.0);

  glPopMatrix();
}

void OGround::drawStartingAreas() const
{
  glPushMatrix();
  btglNormal3(0.0, 0.0, 1.0);
  btglTranslate(0, 0, size_[2]/2+Display::draw_epsilon);

  glColor4fv(color_t1_);
  btglRect(-size_[0]/2, size_[1]/2, -size_[0]/2+start_size_, size_[1]/2-start_size_);
  glColor4fv(color_t2_);
  btglRect(size_[0]/2, size_[1]/2, size_[0]/2-start_size_, size_[1]/2-start_size_);

  glPopMatrix();
}

