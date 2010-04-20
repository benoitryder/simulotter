#include "modules/eurobot2010.h"
#include "physics.h"
#include "config.h"


namespace eurobot2010
{
  const btScalar ORaisedZone::width = btScale(0.500);
  const btScalar ORaisedZone::height = btScale(0.140);
  const btScalar ORaisedZone::bottom_length = btScale(2*0.480+0.500);
  const btScalar ORaisedZone::top_length = btScale(0.500);
  const btScalar ORaisedZone::strip_length = btScale(0.100);
  const btScalar ORaisedZone::wall_width = btScale(0.022);
  const btScalar ORaisedZone::wall_height = btScale(0.070);
  const btScalar ORaisedZone::wall_bottom_length = btScale(2*0.500+0.520);
  const btScalar ORaisedZone::wall_top_length = btScale(0.520);
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
        body_shape_.addPoint( btVector3(-bottom_length/2, -width/2, 0) );
        body_shape_.addPoint( btVector3(-bottom_length/2, +width/2, 0) );
        body_shape_.addPoint( btVector3(+bottom_length/2, +width/2, 0) );
        body_shape_.addPoint( btVector3(+bottom_length/2, -width/2, 0) );
        // top
        body_shape_.addPoint( btVector3(-top_length/2, -width/2, height) );
        body_shape_.addPoint( btVector3(-top_length/2, +width/2, height) );
        body_shape_.addPoint( btVector3(+top_length/2, +width/2, height) );
        body_shape_.addPoint( btVector3(+top_length/2, -width/2, height) );
      }

      if( wall_shape_.getNumPoints() == 0 )
      {
        // front
        wall_shape_.addPoint( btVector3(-wall_bottom_length/2, +wall_width/2, 0) );
        wall_shape_.addPoint( btVector3(+wall_bottom_length/2, +wall_width/2, 0) );
        wall_shape_.addPoint( btVector3(-wall_bottom_length/2, +wall_width/2, wall_height) );
        wall_shape_.addPoint( btVector3(+wall_bottom_length/2, +wall_width/2, wall_height) );
        wall_shape_.addPoint( btVector3(-wall_top_length/2, +wall_width/2, height+wall_height) );
        wall_shape_.addPoint( btVector3(+wall_top_length/2, +wall_width/2, height+wall_height) );
        // back
        wall_shape_.addPoint( btVector3(-wall_bottom_length/2, -wall_width/2, 0) );
        wall_shape_.addPoint( btVector3(+wall_bottom_length/2, -wall_width/2, 0) );
        wall_shape_.addPoint( btVector3(-wall_bottom_length/2, -wall_width/2, wall_height) );
        wall_shape_.addPoint( btVector3(+wall_bottom_length/2, -wall_width/2, wall_height) );
        wall_shape_.addPoint( btVector3(-wall_top_length/2, -wall_width/2, height+wall_height) );
        wall_shape_.addPoint( btVector3(+wall_top_length/2, -wall_width/2, height+wall_height) );
      }

      shape_->addChildShape(btTransform::getIdentity(), &body_shape_);

      btTransform tr = btTransform::getIdentity();
      tr.setOrigin( btVector3(0, (width+wall_width)/2, 0) );
      shape_->addChildShape( tr, &wall_shape_);
      tr.setOrigin( -tr.getOrigin() );
      shape_->addChildShape( tr, &wall_shape_);
    }
    setShape( shape_ );
  }

  void ORaisedZone::draw()
  {
    glPushMatrix();
    drawTransform(getTrans());

    // NOTE: there should not have several instances, thus we do not really
    // need a static display list id to share the display list.
    if( dl_id_ != 0 )
      glCallList(dl_id_);
    else
    {
      // new display list
      dl_id_ = glGenLists(1);
      glNewList(dl_id_, GL_COMPILE_AND_EXECUTE);

      glColor4fv(Color4(1)); // RAL 9016

      glBegin(GL_QUAD_STRIP);
      const btVector2 vn_slope = btVector2(height,(bottom_length-top_length)/2).normalized();
      // slope x>0
      btglVertex3( +bottom_length/2, +width/2, 0 );
      btglVertex3( +bottom_length/2, -width/2, 0 );
      btglNormal3( vn_slope.x, 0, vn_slope.y );
      btglVertex3( +   top_length/2, +width/2, height );
      btglVertex3( +   top_length/2, -width/2, height );
      // top
      btglNormal3( 0, 0, 1 );
      btglVertex3( -top_length/2, +width/2, height );
      btglVertex3( -top_length/2, -width/2, height );
      // slope x<0
      btglNormal3( -vn_slope.x, 0, vn_slope.y );
      btglVertex3( -bottom_length/2, +width/2, 0 );
      btglVertex3( -bottom_length/2, -width/2, 0 );
      glEnd();

      glBegin(GL_QUADS);
      // bottom (with strips)
      btglNormal3( 0, 0, 1 );
      btglVertex3( -bottom_length/2-strip_length, -width/2, cfg.draw_epsilon );
      btglVertex3( -bottom_length/2-strip_length, +width/2, cfg.draw_epsilon );
      btglVertex3( +bottom_length/2+strip_length, +width/2, cfg.draw_epsilon );
      btglVertex3( +bottom_length/2+strip_length, -width/2, cfg.draw_epsilon );
      glEnd();

      glTranslatef(0, -(width+wall_width)/2, 0);
      draw_wall();
      glTranslatef(0, +(width+wall_width), 0);
      draw_wall();

      glEndList();
    }

    glPopMatrix();
  }

  void ORaisedZone::draw_wall()
  {
    glColor4fv(Color4(0x14,0x17,0x1c)); // RAL 9017
    const btVector2 vn_slope = btVector2(height,(wall_bottom_length-wall_top_length)/2).normalized();

    // outline
    glBegin(GL_QUAD_STRIP);
    btglVertex3( -wall_bottom_length/2, -wall_width/2, 0 );
    btglVertex3( -wall_bottom_length/2, +wall_width/2, 0 );
    btglNormal3( 0, 0, -1 ); // bottom
    btglVertex3( +wall_bottom_length/2, -wall_width/2, 0 );
    btglVertex3( +wall_bottom_length/2, +wall_width/2, 0 );
    btglNormal3( 1, 0, 0 ); // side x>0
    btglVertex3( +wall_bottom_length/2, -wall_width/2, wall_height );
    btglVertex3( +wall_bottom_length/2, +wall_width/2, wall_height );
    btglNormal3( vn_slope.x, 0, vn_slope.y ); // slope x>0
    btglVertex3( +wall_top_length/2, -wall_width/2, height+wall_height );
    btglVertex3( +wall_top_length/2, +wall_width/2, height+wall_height );
    btglNormal3( 0, 0, 1 ); // top
    btglVertex3( -wall_top_length/2, -wall_width/2, height+wall_height );
    btglVertex3( -wall_top_length/2, +wall_width/2, height+wall_height );
    btglNormal3( -vn_slope.x, 0, vn_slope.y ); // slope x<0
    btglVertex3( -wall_bottom_length/2, -wall_width/2, wall_height );
    btglVertex3( -wall_bottom_length/2, +wall_width/2, wall_height );
    btglNormal3( -1, 0, 0 ); // side x<0
    btglVertex3( -wall_bottom_length/2, -wall_width/2, 0 );
    btglVertex3( -wall_bottom_length/2, +wall_width/2, 0 );
    glEnd();

    // front face
    glBegin(GL_POLYGON);
    btglNormal3( 0, 1, 0 );
    btglVertex3( -wall_bottom_length/2, +wall_width/2, 0 );
    btglVertex3( +wall_bottom_length/2, +wall_width/2, 0 );
    btglVertex3( +wall_bottom_length/2, +wall_width/2, wall_height );
    btglVertex3( +wall_top_length/2, +wall_width/2, height+wall_height );
    btglVertex3( -wall_top_length/2, +wall_width/2, height+wall_height );
    btglVertex3( -wall_bottom_length/2, +wall_width/2, wall_height );
    glEnd();

    // front face
    glBegin(GL_POLYGON);
    btglNormal3( 0, -1, 0 );
    btglVertex3( -wall_bottom_length/2, -wall_width/2, 0 );
    btglVertex3( +wall_bottom_length/2, -wall_width/2, 0 );
    btglVertex3( +wall_bottom_length/2, -wall_width/2, wall_height );
    btglVertex3( +wall_top_length/2, -wall_width/2, height+wall_height );
    btglVertex3( -wall_top_length/2, -wall_width/2, height+wall_height );
    btglVertex3( -wall_bottom_length/2, -wall_width/2, wall_height );
    glEnd();
  }


  SmartPtr<btCylinderShapeZ> OCorn::shape_(new btCylinderShapeZ(btScale(btVector3(0.025,0.025,0.075))));
  const btScalar OCorn::pivot_radius_ = btScale(0.005);
  const btScalar OCorn::pivot_mass_ = 50;
  SmartPtr<btCollisionShape> OCorn::pivot_shape_(new btSphereShape(pivot_radius_));

  OCorn::OCorn(): opivot_(NULL), pivot_attach_(NULL)
  {
    setShape( shape_ );
    /* actual weight and color of official elements differs from rules
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
    pivot_shape_->calculateLocalInertia(pivot_mass_, inertia);
    opivot_->setMassProps(pivot_mass_, inertia);
    opivot_->updateInertiaTensor();
    opivot_->setCenterOfMassTransform( btTransform(
          btMatrix3x3::getIdentity(), btVector3(x, y, pivot_radius_)
          ) );
    physics_->getWorld()->addRigidBody(opivot_);

    // Create the pivot constraint
    pivot_attach_ = new btPoint2PointConstraint(*this, *opivot_,
        btVector3(0, 0, -h), btVector3(0, 0, -0.9*pivot_radius_)
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


  class LuaORaisedZone: public LuaClass<ORaisedZone>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new ORaisedZone());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };


  class LuaOCorn: public LuaClass<OCorn>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OCorn());
      return 0;
    }

    LUA_DEFINE_SET2(plant,plant, LARG_scaled, LARG_scaled);
    LUA_DEFINE_SET0(uproot,uproot);


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
      LUA_CLASS_MEMBER(plant);
      LUA_CLASS_MEMBER(uproot);
    }
  };


  class LuaOCornFake: public LuaClass<OCornFake>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OCornFake());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };


  class LuaOTomato: public LuaClass<OTomato>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OTomato());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };


  class LuaOOrange: public LuaClass<OOrange>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OOrange());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };


  void LuaEurobotModule::do_import(lua_State *L)
  {
    LUA_IMPORT_CLASS_NS(eurobot2010,ORaisedZone);
    LUA_IMPORT_CLASS_NS(eurobot2010,OCorn);
    LUA_IMPORT_CLASS_NS(eurobot2010,OCornFake);
    LUA_IMPORT_CLASS_NS(eurobot2010,OTomato);
    LUA_IMPORT_CLASS_NS(eurobot2010,OOrange);
    LUA_IMPORT_CLASS(Galipeur);
    LUA_IMPORT_CLASS(OGround);
  }

  LUA_REGISTER_SUB_CLASS_NS(eurobot2010,ORaisedZone,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2010,OCorn,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2010,OCornFake,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2010,OTomato,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2010,OOrange,OSimple);

}

