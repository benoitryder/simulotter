#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include "global.h"
#include "object.h"


Object::Object(): btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL))
{
}

void Object::setShape(btCollisionShape *shape)
{
  if( getCollisionShape() != NULL )
    throw(Error("cannot reassign shape"));
  setCollisionShape(shape);
}

void Object::setMass(btScalar mass)
{
  if( getCollisionShape() == NULL )
    throw(Error("object shape must be set to set its mass"));

  btVector3 inertia(0,0,0);
  if( mass )
    getCollisionShape()->calculateLocalInertia(mass, inertia);

  setMassProps(mass, inertia);
  updateInertiaTensor();
}

void Object::addToWorld(Physics *physics)
{
  if( !isInitialized() )
    throw(Error("object must be initialized to be added to a world"));
  physics->getWorld()->addRigidBody(this);
  physics->getObjs().push_back(this);
}


void Object::setPos(const btVector2 &pos)
{
  btVector3 aabbMin, aabbMax;
  this->getAabb(aabbMin, aabbMax);
  setPos( btVector3(pos.x, pos.y, (aabbMax.z()-aabbMin.z())/2 + cfg->drop_epsilon) );
}


void Object::draw()
{
  drawShape(m_worldTransform, m_collisionShape);
}

void Object::drawTransform(const btTransform &transform)
{
  btScalar m[16];
  transform.getOpenGLMatrix(m);
  btglMultMatrix(m);
}

void Object::drawShape(const btTransform &transform, const btCollisionShape *shape)
{
  glPushMatrix();
  drawTransform(transform);

  switch( shape->getShapeType() )
  {
    case COMPOUND_SHAPE_PROXYTYPE:
      {
        const btCompoundShape *compound_shape = static_cast<const btCompoundShape*>(shape);
        for( int i=compound_shape->getNumChildShapes()-1; i>=0; i-- )
          drawShape(
              compound_shape->getChildTransform(i),
              compound_shape->getChildShape(i)
              );
        break;
      }
    case SPHERE_SHAPE_PROXYTYPE:
      {
        const btSphereShape *sphere_shape = static_cast<const btSphereShape*>(shape);
        glutSolidSphere(sphere_shape->getRadius(), cfg->draw_div, cfg->draw_div);
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
        glutSolidCylinder(r, 2*len, cfg->draw_div, cfg->draw_div);
        glutSolidSphere(r, cfg->draw_div, cfg->draw_div);
        btglTranslate(0, 0, 2*len);
        glutSolidSphere(r, cfg->draw_div, cfg->draw_div);
        break;
      }
    case CYLINDER_SHAPE_PROXYTYPE:
      {
        const btCylinderShape *cylinder_shape = static_cast<const btCylinderShape*>(shape);
        const int axis = cylinder_shape->getUpAxis();
        const btScalar r   = cylinder_shape->getRadius();
        //XXX there is not a getHalfHeight() function
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
        glutSolidCylinder(r, 2*len, cfg->draw_div, cfg->draw_div);
        break;
      }
    default:
      throw(Error("drawing not supported for this geometry class"));
      break;
  }

  glPopMatrix();
}



const btScalar OGround::size_start = scale(0.5);

btBoxShape OGround::shape( scale(btVector3(3.0/2, 2.1/2, 0.1/2)) );


OGround::OGround(const Color4 &color, const Color4 &color_t1, const Color4 &color_t2)
{
  setShape( &shape );
  setPos( btVector3(0, 0, -shape.getHalfExtentsWithMargin().getZ()) );

  this->color = color;
  this->color_t1 = color_t1;
  this->color_t2 = color_t2;
}

OGround::~OGround()
{
}


void OGround::draw()
{
  glPushMatrix();

  drawTransform();
  const btVector3 &size = shape.getHalfExtentsWithMargin();

  // Ground

  glPushMatrix();

  glColor3fv(color);
  btglScale(2*size[0], 2*size[1], 2*size[2]);
  glutSolidCube(1.0);

  glPopMatrix();


  // Starting areas

  glPushMatrix();

  glColor3fv(color_t1);
  btglTranslate(-size[0]+size_start/2, size[1]-size_start/2, size[2]);
  btglScale(size_start, size_start, 2*cfg->draw_epsilon);
  glutSolidCube(1.0f);

  glPopMatrix();

  glPushMatrix();

  glColor3fv(color_t2);
  btglTranslate(size[0]-size_start/2, size[1]-size_start/2, size[2]);
  btglScale(size_start, size_start, 2*cfg->draw_epsilon);
  glutSolidCube(1.0f);

  glPopMatrix();

  glPopMatrix();
}


class LuaObject: public LuaClass<Object>
{
  static int _ctor(lua_State *L)
  {
    Object **ud = new_userdata(L);
    *ud = new Object();
    return 0;
  }

  static int set_shape(lua_State *L)
  {
    btCollisionShape *shape;
    shape = *(btCollisionShape **)luaL_checkudata(L, 2, "Shape");
    get_ptr(L)->setShape(shape);
    return 0;
  }

  LUA_DEFINE_SET1(set_mass, setMass, LARG_f);
  LUA_DEFINE_GET(is_initialized, isInitialized);
  LUA_DEFINE_GETN(3, get_pos, getPos);
  LUA_DEFINE_GETN(4, get_rot, getRot);

  static int set_pos(lua_State *L)
  {
    if( lua_isnone(L, 4) )
      get_ptr(L)->setPos( btVector2(LARG_scaled(2), LARG_scaled(3)) );
    else
      get_ptr(L)->setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
    return 0;
  }

  static int set_rot(lua_State *L)
  {
    if( lua_isnone(L, 4) )
#ifdef BT_EULER_DEFAULT_ZYX
      get_ptr(L)->setRot( btQuaternion(LARG_f(2), LARG_f(3), LARG_f(4)) );
#else
      get_ptr(L)->setRot( btQuaternion(LARG_f(4), LARG_f(2), LARG_f(3)) );
#endif
    else
      get_ptr(L)->setRot( btQuaternion(LARG_f(2), LARG_f(3), LARG_f(4), LARG_f(5)) );
    return 0;
  }

  static int add_to_world(lua_State *L)
  {
    get_ptr(L)->addToWorld(physics);
    return 0;
  }

public:
  LuaObject()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(set_shape);
    LUA_REGFUNC(set_mass);
    LUA_REGFUNC(is_initialized);
    LUA_REGFUNC(get_pos);
    LUA_REGFUNC(get_rot);
    LUA_REGFUNC(set_pos);
    LUA_REGFUNC(set_rot);
    LUA_REGFUNC(add_to_world);
  }
};


class LuaObjectColor: public LuaClass<ObjectColor>
{
  static int _ctor(lua_State *L)
  {
    ObjectColor **ud = new_userdata(L);
    *ud = new ObjectColor();
    return 0;
  }

  static int set_color(lua_State *L)
  {
    Color4 color;
    LuaManager::checkcolor(L, 2, color);
    get_ptr(L)->setColor( color );
    return 0;
  }

public:
  LuaObjectColor()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(set_color);
  }
};


class LuaOGround: public LuaClass<OGround>
{
  static int _ctor(lua_State *L)
  {
    OGround **ud = new_userdata(L);

    Color4 color, color_t1, color_t2;
    LuaManager::checkcolor(L, 2, color);
    LuaManager::checkcolor(L, 3, color_t1);
    LuaManager::checkcolor(L, 4, color_t2);
    *ud = new OGround(color, color_t1, color_t2);
    return 0;
  }

public:
  LuaOGround()
  {
    LUA_REGFUNC(_ctor);
  }
};


LUA_REGISTER_BASE_CLASS(Object);
LUA_REGISTER_SUB_CLASS(ObjectColor,Object);
LUA_REGISTER_SUB_CLASS(OGround,Object);

