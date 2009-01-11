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
}

using namespace eurobot2009;

LUA_REGISTER_SUB_CLASS(OColElem,ObjectColor);
LUA_REGISTER_SUB_CLASS(OLintel,ObjectColor);
LUA_REGISTER_SUB_CLASS(ODispenser,ObjectColor);
LUA_REGISTER_SUB_CLASS(OLintelStorage,ObjectColor);

