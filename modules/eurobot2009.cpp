#include <GL/freeglut.h>
#include "modules/eurobot2009.h"
#include "physics.h"
#include "config.h"


namespace eurobot2009
{
  static const btScalar TABLE_HALF_X     = scale(1.50);
  static const btScalar TABLE_HALF_Y     = scale(1.05);
  static const btScalar WALL_HALF_WIDTH  = scale(0.011);


  SmartPtr<btCylinderShapeZ> OColElem::shape(new btCylinderShapeZ(scale(btVector3(0.035,0.035,0.015))));

  OColElem::OColElem()
  {
    setShape( shape );
    setMass( 0.100 );
  }

  SmartPtr<btBoxShape> OLintel::shape(new btBoxShape(scale(btVector3(0.100,0.035,0.015))));
  OLintel::OLintel()
  {
    setShape( shape );
    setMass( 0.300 );
  }


  const btScalar ODispenser::radius = scale(0.040);
  const btScalar ODispenser::height = scale(0.150);
  SmartPtr<btCylinderShapeZ> ODispenser::shape(new btCylinderShapeZ(btVector3(radius,radius,height/2)));

  ODispenser::ODispenser()
  {
    setShape( shape );
    setColor(COLOR_PLEXI);
    m_checkCollideWith = true;
  }

  void ODispenser::setPos(const btVector3 &v, int side)
  {
    btVector3 offset(0,0, height/2);
    switch( side )
    {
      case 0: offset.setY(-radius); break;
      case 1: offset.setX(-radius); break;
      case 2: offset.setY( radius); break;
      case 3: offset.setX( radius); break;
      default:
        throw(Error("invalid value for dispenser side"));
    }
    Object::setPos( v + offset );
  }

  void ODispenser::fill(Object *o, btScalar z)
  {
    btVector3 pos = getPos();
    pos.setZ(z);
    o->setPos(pos);
  }

  void ODispenser::draw()
  {
    glColor4fv(color);
    glPushMatrix();
    drawTransform(m_worldTransform);
    glTranslatef(0, 0, -height/2);
    glutWireCylinder(radius, height, cfg.draw_div, 10);
    glPopMatrix();
  }

  bool ODispenser::checkCollideWithOverride(btCollisionObject *co)
  {
    btRigidBody *o = btRigidBody::upcast(co);
    if( o )
    {
      if( btVector2(this->getPos()-o->getCenterOfMassPosition()).length() <= radius )
        return false;
    }
    
    return btRigidBody::checkCollideWithOverride(co);
  }


  SmartPtr<btCompoundShape> OLintelStorage::shape;
  btBoxShape OLintelStorage::arm_shape( btVector3(WALL_HALF_WIDTH,scale(0.035),WALL_HALF_WIDTH) );
  btBoxShape OLintelStorage::back_shape( btVector3(scale(0.100),WALL_HALF_WIDTH,scale(0.030)) );
  btBoxShape OLintelStorage::bottom_shape( btVector3(scale(0.100),WALL_HALF_WIDTH,scale(0.035)) );

  OLintelStorage::OLintelStorage()
  {
    // First instance: initialize shape
    if( shape == NULL )
    {
      shape = new btCompoundShape();
      btTransform tr = btTransform::getIdentity();
      // Bottom
      tr.setOrigin( btVector3(0, 3*WALL_HALF_WIDTH-scale(0.035), WALL_HALF_WIDTH-scale(0.035)) );
      shape->addChildShape(tr, &bottom_shape);
      // Back
      tr.setOrigin( btVector3(0, scale(0.035)+WALL_HALF_WIDTH, scale(0.030)-WALL_HALF_WIDTH) );
      shape->addChildShape(tr, &back_shape);
      // Left arm
      tr.setOrigin( btVector3(scale(0.100)-WALL_HALF_WIDTH, 0, 0) );
      shape->addChildShape(tr, &arm_shape);
      // Right arm
      tr.setOrigin( btVector3( -(scale(0.100)-WALL_HALF_WIDTH), 0, 0) );
      shape->addChildShape(tr, &arm_shape);
    }

    setShape( shape );
    setColor(Color4::black());
  }

  void OLintelStorage::setPos(btScalar d, int side)
  {
    btScalar x, y;
    switch( side )
    {
      case 0: x = d; y =  TABLE_HALF_Y+scale(0.035); break;
      case 1: y = d; x =  TABLE_HALF_X+scale(0.035); break;
      case 2: x = d; y = -TABLE_HALF_Y-scale(0.035); break;
      case 3: y = d; x = -TABLE_HALF_X-scale(0.035); break;
      default:
        throw(Error("invalid value for lintel storage side"));
    }
    OSimple::setPos( btVector3(x, y, scale(0.070)+WALL_HALF_WIDTH) );
  }

  void OLintelStorage::fill(OLintel *o)
  {
    btVector3 pos = getPos();
    pos[2] += scale(0.015)+WALL_HALF_WIDTH+cfg.drop_epsilon;
    o->setPos(pos);
  }



  const btScalar Galipeur2009::Pachev::width  = scale(0.080);
  const btScalar Galipeur2009::Pachev::height = scale(0.140);
  const btScalar Galipeur2009::Pachev::z_max  = scale(0.080);

  btBoxShape Galipeur2009::Pachev::shape( 0.5*btVector3(width,width,height) );

  Galipeur2009::Galipeur2009(btScalar m): Galipeur(m)
  {
    // Pàchev
    pachev = new Pachev(this);

    btTransform tr_a, tr_b;
    tr_a.setIdentity();
    tr_b.setIdentity();
    tr_a.getBasis().setEulerZYX(0, -M_PI_2, 0);
    tr_b.getBasis().setEulerZYX(0, -M_PI_2, 0);
    tr_a.setOrigin( btVector3(-d_side, 0, -height/2) );
    tr_b.setOrigin( btVector3(scale(0.04), 0, -Pachev::height/2) );
    pachev_link = new btSliderConstraint(*body, *pachev, tr_a, tr_b, true);
    pachev_link->setLowerAngLimit(0);
    pachev_link->setUpperAngLimit(0);

    pachev_link->setPoweredLinMotor(true);
    pachev_link->setTargetLinMotorVelocity(0);
    // always move at full speed: do not limit acceleration
    pachev_link->setMaxLinMotorForce(scale(100.0));
    pachev_link->setLowerLinLimit(0);
    pachev_link->setUpperLinLimit(0);

    pachev_state = PACHEV_RELEASE;
    pachev_moving = false;
  }

  Galipeur2009::~Galipeur2009()
  {
    delete pachev;
    delete pachev_link;
  }

  Galipeur2009::Pachev::Pachev(Galipeur2009 *robot):
    btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
  {
    btVector3 inertia;
    shape.calculateLocalInertia(0.01, inertia);
    setupRigidBody( btRigidBodyConstructionInfo(0.01,NULL,&shape,inertia) );
    this->robot = robot;
    this->resetTrans();
  }

  Galipeur2009::Pachev::~Pachev()
  {
  }

  bool Galipeur2009::Pachev::checkCollideWithOverride(btCollisionObject *co)
  {
    if( co == robot->body )
      return false;
    btRigidBody *o = btRigidBody::upcast(co);
    if( o )
    {
      switch( robot->pachev_state )
      {
        case PACHEV_RELEASE:
          {
            if( btVector2(this->getCenterOfMassPosition() -
                  o->getCenterOfMassPosition()).length() <= width/2 )
              return false;
          }
          break;

        case PACHEV_GRAB:
          {
            btVector3 diff = this->getCenterOfMassPosition() -
              o->getCenterOfMassPosition();
            if( btVector2(diff).length() <= width/2 && diff.z() < height/2 )
            {
              // Check if object is already constrained
              for(int i=0; i < getNumConstraintRefs(); i++)
                if( &getConstraintRef(i)->getRigidBodyB() == o )
                  return false;

              // Move object on pàchev axis
              btTransform tr = o->getCenterOfMassTransform().inverseTimes(this->getCenterOfMassTransform());
              tr.setOrigin( btVector3(0,0,tr.getOrigin().getZ()) );

              btGeneric6DofConstraint *constraint = new btGeneric6DofConstraint(
                  *this, *o, btTransform::getIdentity(), tr, false);
              for(int i=0; i<6; i++)
                constraint->setLimit(i, 0, 0);
              this->robot->physics->getWorld()->addConstraint(constraint, true);
              return false;
            }
          }
          break;

        case PACHEV_EJECT:
          {
            // Push colliding objects outside
            if( !co->isStaticOrKinematicObject() )
            {
              if( btRigidBody::checkCollideWithOverride(co) )
              {
                btVector2 v = this->robot->pachev_link->getFrameOffsetB()
                  * this->getCenterOfMassPosition();
                v.normalize();
                o->translate( -this->robot->pachev_eject_v * v );
              }
              return false;
            }
          }
          break;
      }
    }
    
    return btRigidBody::checkCollideWithOverride(co);
  }

  void Galipeur2009::addToWorld(Physics *physics)
  {
    physics->getWorld()->addRigidBody(pachev);
    physics->getWorld()->addConstraint(pachev_link, true);
    Galipeur::addToWorld(physics);
  }

  void Galipeur2009::removeFromWorld()
  {
    Physics *ph_bak = this->physics;
    releaseObjects();
    Galipeur::removeFromWorld();
    ph_bak->getWorld()->removeConstraint(pachev_link);
    ph_bak->getWorld()->removeRigidBody(pachev);
  }

  void Galipeur2009::draw()
  {
    Galipeur::draw();

    glPushMatrix();
    drawTransform(pachev->getCenterOfMassTransform());
    btglScale(Pachev::width, Pachev::width, Pachev::height);
    glutWireCube(1.0);
    glPopMatrix();
  }


  void Galipeur2009::setTrans(const btTransform &tr)
  {
    body->setCenterOfMassTransform(tr);
    pachev->resetTrans();
  }

  void Galipeur2009::Pachev::resetTrans()
  {
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin( btVector3(-Galipeur2009::d_side-width/2, 0, height/2) );
    this->setCenterOfMassTransform(robot->body->getCenterOfMassTransform()*tr);
  }

  void Galipeur2009::releaseObjects()
  {
    for(int i=pachev->getNumConstraintRefs()-1; i>=0; i--)
    {
      btTypedConstraint *constraint = pachev->getConstraintRef(i);
      if( constraint != pachev_link )
      {
        physics->getWorld()->removeConstraint(constraint);
        delete constraint;
      }
    }
  }


  void Galipeur2009::asserv()
  {
    Galipeur::asserv();

    if( pachev_moving )
    {
      if( btFabs(get_pachev_pos()-target_pachev_pos) < threshold_pachev )
      {
        pachev_link->setTargetLinMotorVelocity(0);
        pachev_link->setLowerLinLimit( target_pachev_pos );
        pachev_link->setUpperLinLimit( target_pachev_pos );

        pachev_moving = false;
      }
    }
  }

  void Galipeur2009::order_pachev_move(btScalar h)
  {
    target_pachev_pos = CLAMP(h,0,Galipeur2009::Pachev::z_max);
    LOG("PACHEV MOVE  %f (%f)", target_pachev_pos, h);

    pachev_link->setLowerLinLimit( 1 );
    pachev_link->setUpperLinLimit( 0 );
    pachev_link->setTargetLinMotorVelocity(
        (h > get_pachev_pos()) ? pachev_v : -pachev_v
        );

    pachev_moving = true;
  }

  void Galipeur2009::order_pachev_release()
  {
    LOG("PACHEV RELEASE");
    releaseObjects();
    pachev_state = PACHEV_RELEASE;
  }

  void Galipeur2009::order_pachev_grab()
  {
    LOG("PACHEV GRAB");
    pachev_state = PACHEV_GRAB;
  }

  void Galipeur2009::order_pachev_eject()
  {
    if( pachev_state != PACHEV_RELEASE )
      order_pachev_release();

    LOG("PACHEV EJECT");
    pachev_state = PACHEV_EJECT;
  }


  class LuaOColElem: public LuaClass<OColElem>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OColElem());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };

  class LuaOLintel: public LuaClass<OLintel>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OLintel());
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
    }
  };

  class LuaODispenser: public LuaClass<ODispenser>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new ODispenser());
      return 0;
    }

    static int set_pos(lua_State *L)
    {
      if( lua_isnone(L, 4) )
        get_ptr(L,1)->setPosAbove( btVector2(LARG_scaled(2), LARG_scaled(3)) );
      else if( lua_isnone(L, 5) )
        get_ptr(L,1)->Object::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
      else
        get_ptr(L,1)->setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)), LARG_i(5));
      return 0;
    }

    static int fill(lua_State *L)
    {
      Object *o = LuaClass<Object>::get_ptr(L,2); //TODO type is not checked
      get_ptr(L,1)->fill(o, LARG_scaled(3));
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
      LUA_CLASS_MEMBER(set_pos);
      LUA_CLASS_MEMBER(fill);
    }
  };

  class LuaOLintelStorage: public LuaClass<OLintelStorage>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new OLintelStorage());
      return 0;
    }

    static int set_pos(lua_State *L)
    {
      // override OSimple::set_pos(dReal,dReal)
      // with OLintelStorage::set_pos(dReal,int)
      if( lua_isnone(L, 4) )
        get_ptr(L,1)->setPos(LARG_scaled(2), LARG_i(3));
      else
        get_ptr(L,1)->Object::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
      return 0;
    }

    static int fill(lua_State *L)
    {
      OLintel *o = LuaClass<OLintel>::get_ptr(L,2);
      get_ptr(L,1)->fill(o);
      return 0;
    }


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
      LUA_CLASS_MEMBER(set_pos);
      LUA_CLASS_MEMBER(fill);
    }
  };

  class LuaGalipeur2009: public LuaClass<Galipeur2009>
  {
    static int _ctor(lua_State *L)
    {
      store_ptr(L, new Galipeur2009(LARG_f(2)));
      return 0;
    }

    LUA_DEFINE_SET1(order_pachev_move, order_pachev_move, LARG_scaled)
    LUA_DEFINE_SET0(order_pachev_release, order_pachev_release)
    LUA_DEFINE_SET0(order_pachev_grab,    order_pachev_grab)
    LUA_DEFINE_SET0(order_pachev_eject,   order_pachev_eject)

    LUA_DEFINE_GET_SCALED(get_pachev_pos, get_pachev_pos)
    LUA_DEFINE_SET1(set_pachev_v, set_pachev_v, LARG_scaled)
    LUA_DEFINE_SET1(set_pachev_eject_v, set_pachev_eject_v, LARG_scaled)
    LUA_DEFINE_SET1(set_threshold_pachev, set_threshold_pachev, LARG_scaled)


    virtual void init_members(lua_State *L)
    {
      LUA_CLASS_MEMBER(_ctor);
      LUA_CLASS_MEMBER(order_pachev_move);
      LUA_CLASS_MEMBER(order_pachev_release);
      LUA_CLASS_MEMBER(order_pachev_grab);
      LUA_CLASS_MEMBER(order_pachev_eject);
      LUA_CLASS_MEMBER(get_pachev_pos);
      LUA_CLASS_MEMBER(set_pachev_v);
      LUA_CLASS_MEMBER(set_threshold_pachev);
      LUA_CLASS_MEMBER(set_pachev_eject_v);
    }
  };

  void LuaEurobotModule::do_import(lua_State *L)
  {
    LUA_IMPORT_CLASS_NS(eurobot2009,OColElem);
    LUA_IMPORT_CLASS_NS(eurobot2009,OLintel);
    LUA_IMPORT_CLASS_NS(eurobot2009,ODispenser);
    LUA_IMPORT_CLASS_NS(eurobot2009,OLintelStorage);
    LUA_IMPORT_CLASS_NS_NAME(eurobot2009,"Galipeur2009","Galipeur");
    LUA_IMPORT_CLASS(OGround);
  }

  LUA_REGISTER_SUB_CLASS_NS(eurobot2009,OColElem,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2009,OLintel,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2009,ODispenser,OSimple);
  LUA_REGISTER_SUB_CLASS_NS(eurobot2009,OLintelStorage,OSimple);

  LUA_REGISTER_SUB_CLASS_NS(eurobot2009,Galipeur2009,Galipeur);

}

