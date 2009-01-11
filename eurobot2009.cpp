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
    setColor((Color4)COLOR_PLEXI);
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
    //XXX gcc does not find the matching method by itself :(
    ObjectColor::setPos( v + offset );
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
    drawTransform();
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
    
    return Object::checkCollideWithOverride(co);
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
    setColor((Color4)COLOR_BLACK);
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

    //XXX gcc does not find the matching method by itself :(
    ObjectColor::setPos( btVector3(x, y, scale(0.070)+WALL_HALF_WIDTH) );
  }

  void OLintelStorage::fill(OLintel *o)
  {
    btVector3 pos = getPos();
    pos[2] += scale(0.015)+WALL_HALF_WIDTH+cfg->drop_epsilon;
    o->setPos(pos);
  }



  const btScalar RORobot::radius = scale(0.150);
  const btScalar RORobot::height = scale(0.200);
  btCompoundShape RORobot::shape;
  btConvexHullShape RORobot::body_shape;
  btBoxShape RORobot::wheel_shape(scale(btVector3(0.015,0.040,0.040)));//TODO

  RORobot::RORobot(btScalar m)
  {
    // First instance: initialize shape
    if( shape.getNumChildShapes() == 0 )
    {
      // Hexagonal body
      if( body_shape.getNumPoints() == 0 )
      {
        btVector2 p = btVector2(radius,0).rotate(M_PI/6);
        for( int i=0; i<6; i++ )
        {
          body_shape.addPoint( btVector3(p.x,p.y,+height/2) );
          body_shape.addPoint( btVector3(p.x,p.y,-height/2) );
          p = p.rotate(M_PI/3);
        }
      }
      shape.addChildShape(btTransform::getIdentity(), &body_shape);

      // Wheels (use boxes instead of cylinders)
      const btVector3 &wheel_size = wheel_shape.getHalfExtentsWithMargin();
      btTransform tr = btTransform::getIdentity();
      btVector2 vw( radius*cos(M_PI/6) + wheel_size.x(), 0 );
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - wheel_size.y())) );
      shape.addChildShape(tr, &wheel_shape);

      vw = vw.rotate(2*M_PI/3);
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - wheel_size.y())) );
      tr.setRotation( btQuaternion(btVector3(0,0,1), 2*M_PI/3) );
      shape.addChildShape(tr, &wheel_shape);

      vw = vw.rotate(-4*M_PI/3);
      tr.setOrigin( btVector3(vw.x, vw.y, -(height/2 - wheel_size.y())) );
      tr.setRotation( btQuaternion(btVector3(0,0,1), -2*M_PI/3) );
      shape.addChildShape(tr, &wheel_shape);
    }

    setShape( &shape );
    setMass(m);

    btVector3 min,max;
    getAabb(min,max);
  }

  RORobot::~RORobot()
  {
  }

  void RORobot::draw()
  {
    glColor4fv(match->getColor(getTeam()));
    glPushMatrix();
    drawTransform();

    btglTranslate(0, 0, -height/2);

    glPushMatrix();

    btglScale(radius, radius, height);
    btVector2 v;

    // Faces

    glBegin(GL_QUADS);

    v = btVector2(1,0).rotate(M_PI/6);
    btVector2 n(1,0); // normal vector
    for( int i=0; i<6; i++ )
    {
      n = n.rotate(M_PI/3);
      btglNormal3(n.x, n.y, 0.0);

      btglVertex3(v.x, v.y, 0.0);
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(M_PI/3);
      btglVertex3(v.x, v.y, 1.0);
      btglVertex3(v.x, v.y, 0.0);
    }

    glEnd();

    // Bottom
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, -1.0);
    v = btVector2(1,0).rotate(M_PI/6);
    for( int i=0; i<6; i++ )
    {
      btglVertex3(v.x, v.y, 0.0);
      v = v.rotate(M_PI/3);
    }
    glEnd();

    // Top
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, 1.0);
    v = btVector2(1,0).rotate(M_PI/6);
    for( int i=0; i<6; i++ )
    {
      btglVertex3(v.x, v.y, 1.0);
      v = v.rotate(M_PI/3);
    }
    glEnd();

    glPopMatrix();

    // Wheels (box shapes, but drawn using cylinders)
    const btVector3 &wheel_size = wheel_shape.getHalfExtentsWithMargin();
    btglTranslate(0, 0, wheel_size.y());
    btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
    btVector2 vw( radius*btCos(M_PI/6), 0 );

    glPushMatrix();
    btglTranslate(0, vw.y, vw.x);
    glutSolidCylinder(wheel_size.y(), 2*wheel_size.x(), cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw = vw.rotate(2*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(-120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(wheel_size.y(), 2*wheel_size.x(), cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw = vw.rotate(-4*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(wheel_size.y(), 2*wheel_size.x(), cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPopMatrix();

    drawDirection();
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
        get_ptr(L)->ObjectColor::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), 0.0) );
      else if( lua_isnone(L, 5) )
        get_ptr(L)->ObjectColor::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
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
      // override ObjectColor::set_pos(dReal,dReal)
      // with OLintelStorageset_pos(dReal,int)
      if( lua_isnone(L, 4) )
        get_ptr(L)->setPos(LARG_scaled(2), LARG_i(3));
      else
        get_ptr(L)->ObjectColor::setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
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

  public:
    LuaRORobot()
    {
      LUA_REGFUNC(_ctor);
    }
  };
}

using namespace eurobot2009;

LUA_REGISTER_SUB_CLASS(OColElem,ObjectColor);
LUA_REGISTER_SUB_CLASS(OLintel,ObjectColor);
LUA_REGISTER_SUB_CLASS(ODispenser,ObjectColor);
LUA_REGISTER_SUB_CLASS(OLintelStorage,ObjectColor);

LUA_REGISTER_SUB_CLASS(RORobot,RBasic);

