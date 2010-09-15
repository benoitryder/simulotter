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
        const btScalar r   = cylinder_shape->getRadius();
        // there is not a getHalfHeight() function
        const btVector3 &size = cylinder_shape->getHalfExtentsWithMargin();
        const btScalar len = size[axis];
        switch( axis )
        {
          case 0: btglRotate(-90.0, 0.0, 1.0, 0.0); break;
          case 1: btglRotate(-90.0, 1.0, 0.0, 0.0); break;
          case 2: break;
          default:
            throw(Error("invalid capsule up axis"));
        }
        btglTranslate(0, 0, -len);
        glutSolidCylinder(r, 2*len, Display::draw_div, Display::draw_div);
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
  if( physics->getObjs().insert(this).second == false )
    throw(Error("object added to the world twice"));
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
  btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
{
}

OSimple::OSimple(btCollisionShape *shape, btScalar mass):
  btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
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
  if( getCollisionShape() != NULL )
    throw(Error("cannot reassign shape"));
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
  setPos( btVector3(pos.x(), pos.y(), (aabbMax.z()-aabbMin.z())/2 + Physics::margin_epsilon) );
}


void OSimple::draw(Display *d)
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


const btScalar OGround::SIZE_START = btScale(0.5);
SmartPtr<btBoxShape> OGround::shape_( new btBoxShape( btScale(btVector3(3.0/2, 2.1/2, 0.1/2)) ) );

OGround::OGround(const Color4 &color, const Color4 &color_t1, const Color4 &color_t2)
{
  setShape( shape_ );
  setPos( btVector3(0, 0, -shape_->getHalfExtentsWithMargin().getZ()) );
  setColor(color);
  color_t1_ = color_t1;
  color_t2_ = color_t2;
}

OGround::~OGround()
{
}


void OGround::draw(Display *d)
{
  glPushMatrix();

  drawTransform(m_worldTransform);

  if( d->callOrCreateDisplayList(this) ) {
    // Their should be only one ground instance, thus we create one display
    // list per instance. This allow to put color changes in it.

    const btVector3 &size = shape_->getHalfExtentsWithMargin();

    // Ground

    glPushMatrix();

    glColor4fv(color_);
    btglScale(2*size[0], 2*size[1], 2*size[2]);
    glutSolidCube(1.0);

    glPopMatrix();


    // Starting areas

    glPushMatrix();

    glColor4fv(color_t1_);
    btglTranslate(-size[0]+SIZE_START/2, size[1]-SIZE_START/2, size[2]);
    btglScale(SIZE_START, SIZE_START, 2*Display::draw_epsilon);
    glutSolidCube(1.0f);

    glPopMatrix();

    glPushMatrix();

    glColor4fv(color_t2_);
    btglTranslate(size[0]-SIZE_START/2, size[1]-SIZE_START/2, size[2]);
    btglScale(SIZE_START, SIZE_START, 2*Display::draw_epsilon);
    glutSolidCube(1.0f);

    glPopMatrix();

    d->endDisplayList();
  }

  glPopMatrix();
}


