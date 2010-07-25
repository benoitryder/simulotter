#include "modules/eurobot2010.h"
#include "physics.h"
#include "display.h"
#include "log.h"


namespace eurobot2010
{
  const btScalar ORaisedZone::WIDTH = btScale(0.500);
  const btScalar ORaisedZone::HEIGHT = btScale(0.140);
  const btScalar ORaisedZone::BOTTOM_LENGTH = btScale(2*0.480+0.500);
  const btScalar ORaisedZone::TOP_LENGTH = btScale(0.500);
  const btScalar ORaisedZone::STRIP_LENGTH = btScale(0.100);
  const btScalar ORaisedZone::WALL_WIDTH = btScale(0.022);
  const btScalar ORaisedZone::WALL_HEIGHT = btScale(0.070);
  const btScalar ORaisedZone::WALL_BOTTOM_LENGTH = btScale(2*0.500+0.520);
  const btScalar ORaisedZone::WALL_TOP_LENGTH = btScale(0.520);
  SmartPtr<btCompoundShape> ORaisedZone::shape_;
  btConvexHullShape ORaisedZone::body_shape_;
  btConvexHullShape ORaisedZone::wall_shape_;

  ORaisedZone::ORaisedZone()
  {
    // First instance: initialize shape
    if( shape_ == NULL )
    {
      shape_ = new btCompoundShape();

      if( body_shape_.getNumPoints() == 0 )
      {
        // bottom
        body_shape_.addPoint( btVector3(-BOTTOM_LENGTH/2, -WIDTH/2, 0) );
        body_shape_.addPoint( btVector3(-BOTTOM_LENGTH/2, +WIDTH/2, 0) );
        body_shape_.addPoint( btVector3(+BOTTOM_LENGTH/2, +WIDTH/2, 0) );
        body_shape_.addPoint( btVector3(+BOTTOM_LENGTH/2, -WIDTH/2, 0) );
        // top
        body_shape_.addPoint( btVector3(-TOP_LENGTH/2, -WIDTH/2, HEIGHT) );
        body_shape_.addPoint( btVector3(-TOP_LENGTH/2, +WIDTH/2, HEIGHT) );
        body_shape_.addPoint( btVector3(+TOP_LENGTH/2, +WIDTH/2, HEIGHT) );
        body_shape_.addPoint( btVector3(+TOP_LENGTH/2, -WIDTH/2, HEIGHT) );
      }

      if( wall_shape_.getNumPoints() == 0 )
      {
        // front
        wall_shape_.addPoint( btVector3(-WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0) );
        wall_shape_.addPoint( btVector3(+WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0) );
        wall_shape_.addPoint( btVector3(-WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(+WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(-WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(+WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT) );
        // back
        wall_shape_.addPoint( btVector3(-WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0) );
        wall_shape_.addPoint( btVector3(+WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0) );
        wall_shape_.addPoint( btVector3(-WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(+WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(-WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT) );
        wall_shape_.addPoint( btVector3(+WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT) );
      }

      shape_->addChildShape(btTransform::getIdentity(), &body_shape_);

      btTransform tr = btTransform::getIdentity();
      tr.setOrigin( btVector3(0, (WIDTH+WALL_WIDTH)/2, 0) );
      shape_->addChildShape( tr, &wall_shape_);
      tr.setOrigin( -tr.getOrigin() );
      shape_->addChildShape( tr, &wall_shape_);
    }
    setShape( shape_ );
  }

  void ORaisedZone::draw(Display *d)
  {
    glPushMatrix();
    drawTransform(getTrans());

    if( d->callOrCreateDisplayList(this) ) {
      // There should not have several instances, thus we do not really need to
      // share the display list.

      glColor4fv(Color4(1)); // RAL 9016

      glBegin(GL_QUAD_STRIP);
      const btVector2 vn_slope = btVector2(HEIGHT,(BOTTOM_LENGTH-TOP_LENGTH)/2).normalized();
      // slope x>0
      btglVertex3( +BOTTOM_LENGTH/2, +WIDTH/2, 0 );
      btglVertex3( +BOTTOM_LENGTH/2, -WIDTH/2, 0 );
      btglNormal3( vn_slope.x(), 0, vn_slope.y() );
      btglVertex3( +   TOP_LENGTH/2, +WIDTH/2, HEIGHT );
      btglVertex3( +   TOP_LENGTH/2, -WIDTH/2, HEIGHT );
      // top
      btglNormal3( 0, 0, 1 );
      btglVertex3( -TOP_LENGTH/2, +WIDTH/2, HEIGHT );
      btglVertex3( -TOP_LENGTH/2, -WIDTH/2, HEIGHT );
      // slope x<0
      btglNormal3( -vn_slope.x(), 0, vn_slope.y() );
      btglVertex3( -BOTTOM_LENGTH/2, +WIDTH/2, 0 );
      btglVertex3( -BOTTOM_LENGTH/2, -WIDTH/2, 0 );
      glEnd();

      glBegin(GL_QUADS);
      // bottom (with strips)
      btglNormal3( 0, 0, 1 );
      btglVertex3( -BOTTOM_LENGTH/2-STRIP_LENGTH, -WIDTH/2, Display::draw_epsilon );
      btglVertex3( -BOTTOM_LENGTH/2-STRIP_LENGTH, +WIDTH/2, Display::draw_epsilon );
      btglVertex3( +BOTTOM_LENGTH/2+STRIP_LENGTH, +WIDTH/2, Display::draw_epsilon );
      btglVertex3( +BOTTOM_LENGTH/2+STRIP_LENGTH, -WIDTH/2, Display::draw_epsilon );
      glEnd();

      glTranslatef(0, -(WIDTH+WALL_WIDTH)/2, 0);
      draw_wall();
      glTranslatef(0, +(WIDTH+WALL_WIDTH), 0);
      draw_wall();

      d->endDisplayList();
    }

    glPopMatrix();
  }

  void ORaisedZone::draw_wall()
  {
    glColor4fv(Color4(0x14,0x17,0x1c)); // RAL 9017
    const btVector2 vn_slope = btVector2(HEIGHT,(WALL_BOTTOM_LENGTH-WALL_TOP_LENGTH)/2).normalized();

    // outline
    glBegin(GL_QUAD_STRIP);
    btglVertex3( -WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0 );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0 );
    btglNormal3( 0, 0, -1 ); // bottom
    btglVertex3( +WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0 );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0 );
    btglNormal3( 1, 0, 0 ); // side x>0
    btglVertex3( +WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT );
    btglNormal3( vn_slope.x(), 0, vn_slope.y() ); // slope x>0
    btglVertex3( +WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( +WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglNormal3( 0, 0, 1 ); // top
    btglVertex3( -WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( -WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglNormal3( -vn_slope.x(), 0, vn_slope.y() ); // slope x<0
    btglVertex3( -WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT );
    btglNormal3( -1, 0, 0 ); // side x<0
    btglVertex3( -WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0 );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0 );
    glEnd();

    // front face
    glBegin(GL_POLYGON);
    btglNormal3( 0, 1, 0 );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0 );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, 0 );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT );
    btglVertex3( +WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( -WALL_TOP_LENGTH/2, +WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, +WALL_WIDTH/2, WALL_HEIGHT );
    glEnd();

    // front face
    glBegin(GL_POLYGON);
    btglNormal3( 0, -1, 0 );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0 );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, 0 );
    btglVertex3( +WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT );
    btglVertex3( +WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( -WALL_TOP_LENGTH/2, -WALL_WIDTH/2, HEIGHT+WALL_HEIGHT );
    btglVertex3( -WALL_BOTTOM_LENGTH/2, -WALL_WIDTH/2, WALL_HEIGHT );
    glEnd();
  }


  SmartPtr<btCylinderShapeZ> OCorn::shape_(new btCylinderShapeZ(btScale(btVector3(0.025,0.025,0.075))));
  const btScalar OCorn::PIVOT_RADIUS = btScale(0.005);
  const btScalar OCorn::PIVOT_MASS = 50;
  SmartPtr<btCollisionShape> OCorn::pivot_shape_(new btSphereShape(PIVOT_RADIUS));

  OCorn::OCorn(): opivot_(NULL), pivot_attach_(NULL)
  {
    setShape( shape_ );
    /* actual weight and color of official elements differ from rules
    setMass( 0.250 );
    setColor(Color4( 0xff, 0xf5, 0xe3 )); // RAL 1013
    */
    setMass( 0.285 );
    setColor(Color4( 0xd0, 0xd0, 0xd0 )); // gray
  }

  void OCorn::plant(btScalar x, btScalar y)
  {
    if( physics_ == NULL )
      throw(Error("cannot plant a corn which is not in a world"));

    uproot();

    btScalar h = shape_->getHalfExtentsWithMargin().getZ();

    // Create the pivot rigid body.
    opivot_ = new btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL));
    opivot_->setCollisionShape(pivot_shape_);
    btVector3 inertia;
    pivot_shape_->calculateLocalInertia(PIVOT_MASS, inertia);
    opivot_->setMassProps(PIVOT_MASS, inertia);
    opivot_->updateInertiaTensor();
    opivot_->setCenterOfMassTransform( btTransform(
          btMatrix3x3::getIdentity(), btVector3(x, y, PIVOT_RADIUS)
          ) );
    physics_->getWorld()->addRigidBody(opivot_);

    // Create the pivot constraint
    pivot_attach_ = new btPoint2PointConstraint(*this, *opivot_,
        btVector3(0, 0, -h), btVector3(0, 0, -0.9*PIVOT_RADIUS)
        );
    physics_->getWorld()->addConstraint(pivot_attach_, true);

    enableTickCallback();

    setPos( btVector3( x, y, h ) );
  }

  void OCorn::uproot()
  {
    if( !physics_ || !pivot_attach_ )
      return;

    physics_->getWorld()->removeConstraint(pivot_attach_);
    delete pivot_attach_;

    pivot_attach_ = NULL;
    physics_->getWorld()->removeRigidBody(opivot_);
    delete opivot_;
    opivot_ = NULL;

    disableTickCallback();
  }

  void OCorn::removeFromWorld()
  {
    uproot();
    OSimple::removeFromWorld();
  }

  void OCorn::tickCallback()
  {
    // remove constraint if corn angle is not null
    btQuaternion q;
    getRot().getRotation(q);
    if( btFabs(q.getAngle()) > 0.1 )
      uproot();
  }


  SmartPtr<btCylinderShapeZ> OCornFake::shape_(new btCylinderShapeZ(btScale(btVector3(0.025,0.025,0.075))));

  OCornFake::OCornFake()
  {
    setShape( shape_ );
    setMass(0);
    setColor(Color4( 0x14, 0x17, 0x1c )); // RAL 9017
  }


  SmartPtr<btSphereShape> OTomato::shape_(new btSphereShape(btScale(0.050)));

  OTomato::OTomato()
  {
    setShape( shape_ );
    setColor( Color4(0xff, 0,0) );
    setMass( 0.150 );
  }


  SmartPtr<btSphereShape> OOrange::shape_(new btSphereShape(btScale(0.050)));

  OOrange::OOrange()
  {
    setShape( shape_ );
    setColor( Color4(0xff, 0x80,0) );
    setMass( 0.300 );
  }

}

