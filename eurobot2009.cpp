#include <GL/freeglut.h>
#include "eurobot2009.h"


namespace eurobot2009
{
  static const btScalar TABLE_HALF_X     = scale(1.50);
  static const btScalar TABLE_HALF_Y     = scale(1.05);
  static const btScalar WALL_HALF_WIDTH  = scale(0.011);


  btCylinderShapeZ OColElem::shape( scale(btVector3(0.035,0.035,0.015)) );

  OColElem::OColElem()
  {
    setShape( &shape );
    setMass( 0.100 );
  }

  btBoxShape OLintel::shape( scale(btVector3(0.100,0.035,0.015)) );
  OLintel::OLintel()
  {
    setShape( &shape );
    setMass( 0.300 );
  }


  const btScalar ODispenser::radius = scale(0.040);
  const btScalar ODispenser::height = scale(0.150);
  btCylinderShapeZ ODispenser::shape( btVector3(radius,radius,height/2) );

  ODispenser::ODispenser()
  {
    setShape( &shape );
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
    glutWireCylinder(radius, height, cfg->draw_div, 10);
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


  btCompoundShape OLintelStorage::shape;
  btBoxShape OLintelStorage::arm_shape( btVector3(WALL_HALF_WIDTH,scale(0.035),WALL_HALF_WIDTH) );
  btBoxShape OLintelStorage::back_shape( btVector3(scale(0.100),WALL_HALF_WIDTH,scale(0.030)) );
  btBoxShape OLintelStorage::bottom_shape( btVector3(scale(0.100),WALL_HALF_WIDTH,scale(0.035)) );

  OLintelStorage::OLintelStorage()
  {
    // First instance: initialize shape
    if( shape.getNumChildShapes() == 0 )
    {
      btTransform tr = btTransform::getIdentity();
      // Bottom
      tr.setOrigin( btVector3(0, 3*WALL_HALF_WIDTH-scale(0.035), WALL_HALF_WIDTH-scale(0.035)) );
      shape.addChildShape(tr, &bottom_shape);
      // Back
      tr.setOrigin( btVector3(0, scale(0.035)+WALL_HALF_WIDTH, scale(0.030)-WALL_HALF_WIDTH) );
      shape.addChildShape(tr, &back_shape);
      // Left arm
      tr.setOrigin( btVector3(scale(0.100)-WALL_HALF_WIDTH, 0, 0) );
      shape.addChildShape(tr, &arm_shape);
      // Right arm
      tr.setOrigin( btVector3( -(scale(0.100)-WALL_HALF_WIDTH), 0, 0) );
      shape.addChildShape(tr, &arm_shape);
    }

    setShape( &shape );
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
    pos[2] += scale(0.015)+WALL_HALF_WIDTH+cfg->drop_epsilon;
    o->setPos(pos);
  }



  const btScalar RORobot::height   = scale(0.200);
  const btScalar RORobot::side     = scale(0.110);
  const btScalar RORobot::r_wheel  = scale(0.025);
  const btScalar RORobot::h_wheel  = scale(0.020);
  const btScalar RORobot::Pachev::width  = scale(0.080);
  const btScalar RORobot::Pachev::height = scale(0.140);
  const btScalar RORobot::Pachev::z_max  = scale(0.080);

  const btScalar RORobot::d_side   = ( side + 2*r_wheel ) / btSqrt(3);
  const btScalar RORobot::d_wheel  = ( r_wheel + 2*side ) / btSqrt(3);
  const btScalar RORobot::a_side   = 2*btAtan2( side,    d_side  );
  const btScalar RORobot::a_wheel  = 2*btAtan2( r_wheel, d_wheel );
  const btScalar RORobot::radius   = btSqrt(side*side+d_side*d_side);

  btCompoundShape RORobot::shape;
  btConvexHullShape RORobot::body_shape;
  btBoxShape RORobot::wheel_shape( btVector3(h_wheel/2,r_wheel,r_wheel) );
  btBoxShape RORobot::Pachev::shape( 0.5*btVector3(width,width,height) );

  RORobot::RORobot(btScalar m)
  {
    // First instance: initialize shape
    if( shape.getNumChildShapes() == 0 )
    {
      // Triangular body
      if( body_shape.getNumPoints() == 0 )
      {
        btVector2 p = btVector2(radius,0).rotate(-a_wheel/2);
        for( int i=0; i<3; i++ )
        {
          body_shape.addPoint( btVector3(p.x,p.y,+height/2) );
          body_shape.addPoint( btVector3(p.x,p.y,-height/2) );
          p = p.rotate(a_wheel);
          body_shape.addPoint( btVector3(p.x,p.y,+height/2) );
          body_shape.addPoint( btVector3(p.x,p.y,-height/2) );
          p = p.rotate(a_side);
        }
      }
      shape.addChildShape(btTransform::getIdentity(), &body_shape);

      // Wheels (use boxes instead of cylinders)
      btTransform tr = btTransform::getIdentity();
      btVector2 vw( d_wheel+h_wheel/2, 0 );
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - r_wheel)) );
      shape.addChildShape(tr, &wheel_shape);

      vw = vw.rotate(2*M_PI/3);
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - r_wheel)) );
      tr.setRotation( btQuaternion(btVector3(0,0,1), 2*M_PI/3) );
      shape.addChildShape(tr, &wheel_shape);

      vw = vw.rotate(-4*M_PI/3);
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - r_wheel)) );
      tr.setRotation( btQuaternion(btVector3(0,0,1), -2*M_PI/3) );
      shape.addChildShape(tr, &wheel_shape);
    }

    this->setup(&shape, m);

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
  }

  RORobot::~RORobot()
  {
    //TODO remove elements from the world
    delete pachev;
    delete pachev_link;
  }

  RORobot::Pachev::Pachev(RORobot *robot):
    btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
  {
    btVector3 inertia;
    shape.calculateLocalInertia(0.01, inertia);
    setupRigidBody( btRigidBodyConstructionInfo(0.01,NULL,&shape,inertia) );
    this->robot = robot;
    this->resetTrans();
  }

  RORobot::Pachev::~Pachev()
  {
    //TODO remove constraints (?)
  }

  bool RORobot::Pachev::checkCollideWithOverride(btCollisionObject *co)
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
              physics->getWorld()->addConstraint(constraint, true);
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
                //TODO use proper coeff with pachev_eject_v
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

  void RORobot::addToWorld(Physics *physics)
  {
    RBasic::addToWorld(physics);
    physics->getWorld()->addRigidBody(pachev);
    physics->getWorld()->addConstraint(pachev_link, true);
  }

  void RORobot::draw()
  {
    // Use a darker color to constrat with game elements
    glColor4fv(match->getColor(getTeam()) * 0.5);

    glPushMatrix();
    drawTransform(pachev->getCenterOfMassTransform());
    btglScale(Pachev::width, Pachev::width, Pachev::height);
    glutWireCube(1.0);
    glPopMatrix();

    glPushMatrix();
    drawTransform(body->getCenterOfMassTransform());

    btglTranslate(0, 0, -height/2);

    glPushMatrix();

    btglScale(radius, radius, height);
    btVector2 v;

    // Faces

    glBegin(GL_QUADS);

    v = btVector2(1,0).rotate(-a_wheel/2);
    btVector2 n(1,0); // normal vector
    for( int i=0; i<3; i++ )
    {
      // wheel side
      btglNormal3(n.x, n.y, 0.0);
      n = n.rotate(M_PI/3);

      btglVertex3(v.x, v.y, 0.0);
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 1.0);
      btglVertex3(v.x, v.y, 0.0);

      // triangle side
      btglNormal3(n.x, n.y, 0.0);
      n = n.rotate(M_PI/3);

      btglVertex3(v.x, v.y, 0.0);
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(a_side);
      btglVertex3(v.x, v.y, 1.0);
      btglVertex3(v.x, v.y, 0.0);
    }

    glEnd();

    // Bottom
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, -1.0);
    v = btVector2(1,0).rotate(-a_wheel/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x, v.y, 0.0);
      v = v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 0.0);
      v = v.rotate(a_side);
    }
    glEnd();

    // Top
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, 1.0);
    v = btVector2(1,0).rotate(-a_wheel/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(a_side);
    }
    glEnd();

    glPopMatrix();

    // Wheels (box shapes, but drawn using cylinders)
    btglTranslate(0, 0, r_wheel);
    btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
    btVector2 vw( d_wheel, 0 );

    glPushMatrix();
    btglTranslate(0, vw.y, vw.x);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw = vw.rotate(2*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(-120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw = vw.rotate(-4*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPopMatrix();

    drawDirection();
  }


  void RORobot::setTrans(const btTransform &tr)
  {
    body->setCenterOfMassTransform(tr);
    pachev->resetTrans();
  }

  void RORobot::Pachev::resetTrans()
  {
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin( btVector3(-RORobot::d_side-width/2, 0, height/2) );
    this->setCenterOfMassTransform(robot->body->getCenterOfMassTransform()*tr);
  }


  void RORobot::do_asserv()
  {
    RBasic::do_asserv();

    if( order & ORDER_PACHEV_MOVE )
    {
      if( btFabs(get_pachev_pos()-target_pachev_pos) < threshold_pachev )
      {
        pachev_link->setTargetLinMotorVelocity(0);
        pachev_link->setLowerLinLimit( target_pachev_pos );
        pachev_link->setUpperLinLimit( target_pachev_pos );

        order &= ~ORDER_PACHEV_MOVE;
      }
    }
  }

  void RORobot::order_pachev_move(btScalar h)
  {
    target_pachev_pos = CLAMP(h,0,RORobot::Pachev::z_max);
    LOG->trace("PACHEV MOVE  %f (%f)", target_pachev_pos, h);

    pachev_link->setLowerLinLimit( 1 );
    pachev_link->setUpperLinLimit( 0 );
    pachev_link->setTargetLinMotorVelocity(
        (h > get_pachev_pos()) ? pachev_v : -pachev_v
        );

    order |= ORDER_PACHEV_MOVE;
  }

  void RORobot::order_pachev_release()
  {
    LOG->trace("PACHEV RELEASE");
    for(int i=pachev->getNumConstraintRefs()-1; i>=0; i--)
    {
      btTypedConstraint *constraint = pachev->getConstraintRef(i);
      if( &constraint->getRigidBodyA() != body )
      {
        physics->getWorld()->removeConstraint(constraint);
        delete constraint;
      }
    }
    pachev_state = PACHEV_RELEASE;
  }

  void RORobot::order_pachev_grab()
  {
    LOG->trace("PACHEV GRAB");
    pachev_state = PACHEV_GRAB;
  }

  void RORobot::order_pachev_eject()
  {
    if( pachev_state != PACHEV_RELEASE )
      order_pachev_release();

    LOG->trace("PACHEV EJECT");
    pachev_state = PACHEV_EJECT;
  }


  class LuaOColElem: public LuaClass<OColElem>
  {
    static int _ctor(lua_State *L)
    {
      OColElem **ud = new_userdata(L);
      *ud = new OColElem();
      return 0;
    }

  public:
    LuaOColElem()
    {
      LUA_REGFUNC(_ctor);
    }
  };

  class LuaOLintel: public LuaClass<OLintel>
  {
    static int _ctor(lua_State *L)
    {
      OLintel **ud = new_userdata(L);
      *ud = new OLintel();
      return 0;
    }

  public:
    LuaOLintel()
    {
      LUA_REGFUNC(_ctor);
    }
  };

  class LuaODispenser: public LuaClass<ODispenser>
  {
    static int _ctor(lua_State *L)
    {
      ODispenser **ud = new_userdata(L);
      *ud = new ODispenser();
      return 0;
    }

    static int set_pos(lua_State *L)
    {
      if( lua_isnone(L, 4) )
        get_ptr(L)->setPosAbove( btVector2(LARG_scaled(2), LARG_scaled(3)) );
      else if( lua_isnone(L, 5) )
        get_ptr(L)->Object::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
      else
        get_ptr(L)->setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)), LARG_i(5));
      return 0;
    }

    static int fill(lua_State *L)
    {
      lua_getfield(L, 2, "_ud");
      //XXX check validity
      Object *o = *(Object**)lua_touserdata(L, -1);
      lua_pop(L, 1);
      get_ptr(L)->fill(o, LARG_scaled(3));
      return 0;
    }

  public:
    LuaODispenser()
    {
      LUA_REGFUNC(_ctor);
      LUA_REGFUNC(set_pos);
      LUA_REGFUNC(fill);
    }
  };

  class LuaOLintelStorage: public LuaClass<OLintelStorage>
  {
    static int _ctor(lua_State *L)
    {
      OLintelStorage **ud = new_userdata(L);
      *ud = new OLintelStorage();
      return 0;
    }

    static int set_pos(lua_State *L)
    {
      // override OSimple::set_pos(dReal,dReal)
      // with OLintelStorage::set_pos(dReal,int)
      if( lua_isnone(L, 4) )
        get_ptr(L)->setPos(LARG_scaled(2), LARG_i(3));
      else
        get_ptr(L)->Object::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
      return 0;
    }

    static int fill(lua_State *L)
    {
      lua_getfield(L, 2, "_ud");
      //XXX check validity
      OLintel *o = *(OLintel**)lua_touserdata(L, -1);
      lua_pop(L, 1);
      get_ptr(L)->fill(o);
      return 0;
    }

  public:
    LuaOLintelStorage()
    {
      LUA_REGFUNC(_ctor);
      LUA_REGFUNC(set_pos);
      LUA_REGFUNC(fill);
    }
  };

  class LuaRORobot: public LuaClass<RORobot>
  {
    static int _ctor(lua_State *L)
    {
      RORobot **ud = new_userdata(L);
      *ud = new RORobot(LARG_f(2));
      lua_pushvalue(L, 1);
      (*ud)->ref_obj = luaL_ref(L, LUA_REGISTRYINDEX);

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

  public:
    LuaRORobot()
    {
      LUA_REGFUNC(_ctor);
      LUA_REGFUNC(order_pachev_move);
      LUA_REGFUNC(order_pachev_release);
      LUA_REGFUNC(order_pachev_grab);
      LUA_REGFUNC(order_pachev_eject);
      LUA_REGFUNC(get_pachev_pos);
      LUA_REGFUNC(set_pachev_v);
      LUA_REGFUNC(set_threshold_pachev);
      LUA_REGFUNC(set_pachev_eject_v);
    }
  };
}

using namespace eurobot2009;

LUA_REGISTER_SUB_CLASS(OColElem,OSimple);
LUA_REGISTER_SUB_CLASS(OLintel,OSimple);
LUA_REGISTER_SUB_CLASS(ODispenser,OSimple);
LUA_REGISTER_SUB_CLASS(OLintelStorage,OSimple);

LUA_REGISTER_SUB_CLASS(RORobot,RBasic);

