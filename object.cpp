#include <SDL/SDL.h>
#include <GL/freeglut.h>
#include "global.h"
#include "object.h"


void Object::drawTransform(const btTransform &transform)
{
  btScalar m[16];
  transform.getOpenGLMatrix(m);
  btglMultMatrix(m);
}

void Object::drawShape(const btCollisionShape *shape)
{
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
}

GLuint Object::createDisplayList(const btCollisionShape *shape)
{
  GLuint dl_id = glGenLists(1);
  glNewList(dl_id, GL_COMPILE);
  drawShape(shape);
  glEndList();
  return dl_id;
}

void Object::addToWorld(Physics *physics)
{
  if( physics->getObjs().insert(this).second == false )
    throw(Error("object added to the world twice"));
}


std::map<const btCollisionShape *, GLuint> OSimple::shape2dl;

OSimple::OSimple():
  btRigidBody(btRigidBodyConstructionInfo(0,NULL,NULL)),
  dl_id(0)
{
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


void OSimple::setPosAbove(const btVector2 &pos)
{
  btVector3 aabbMin, aabbMax;
  this->getAabb(aabbMin, aabbMax);
  setPos( btVector3(pos.x, pos.y, (aabbMax.z()-aabbMin.z())/2 + cfg->drop_epsilon) );
}


void OSimple::draw()
{
  glColor4fv(color);
  glPushMatrix();
  drawTransform(m_worldTransform);

  if( dl_id == 0 )
  {
    std::map<const btCollisionShape *, GLuint>::iterator it;
    const btCollisionShape *shape = m_collisionShape;
    it = shape2dl.find( shape );
    if( it == shape2dl.end() )
      shape2dl[shape] = dl_id = createDisplayList(shape);
    else
      dl_id = (*it).second;
  }
  glCallList(dl_id);

  glPopMatrix();
}


const btScalar OGround::size_start = scale(0.5);
SmartPtr<btBoxShape> OGround::shape( new btBoxShape( scale(btVector3(3.0/2, 2.1/2, 0.1/2)) ) );

OGround::OGround(const Color4 &color, const Color4 &color_t1, const Color4 &color_t2)
{
  setShape( shape );
  setPos( btVector3(0, 0, -shape->getHalfExtentsWithMargin().getZ()) );
  setColor(color);
  this->color_t1 = color_t1;
  this->color_t2 = color_t2;
}

OGround::~OGround()
{
  if( dl_id != 0 )
    glDeleteLists(dl_id, 1);
  dl_id = 0;
}


void OGround::draw()
{
  glPushMatrix();

  drawTransform(m_worldTransform);

  if( dl_id != 0 )
    glCallList(dl_id);
  else
  {
    // Create the display list
    // Their should be only one ground instance, thus we create one display
    // list per instance. This allow to put color changes in it.

    const btVector3 &size = shape->getHalfExtentsWithMargin();

    dl_id = glGenLists(1);
    glNewList(dl_id, GL_COMPILE_AND_EXECUTE);

    // Ground

    glPushMatrix();

    glColor4fv(color);
    btglScale(2*size[0], 2*size[1], 2*size[2]);
    glutSolidCube(1.0);

    glPopMatrix();


    // Starting areas

    glPushMatrix();

    glColor4fv(color_t1);
    btglTranslate(-size[0]+size_start/2, size[1]-size_start/2, size[2]);
    btglScale(size_start, size_start, 2*cfg->draw_epsilon);
    glutSolidCube(1.0f);

    glPopMatrix();

    glPushMatrix();

    glColor4fv(color_t2);
    btglTranslate(size[0]-size_start/2, size[1]-size_start/2, size[2]);
    btglScale(size_start, size_start, 2*cfg->draw_epsilon);
    glutSolidCube(1.0f);

    glPopMatrix();

    glEndList();
  }

  glPopMatrix();
}


class LuaObject: public LuaClass<Object>
{
  static int _ctor(lua_State *L)
  {
    return luaL_error(L, "Object class is abstract, no constructor");
  }

  LUA_DEFINE_GETN(3, get_pos, getPos);
  LUA_DEFINE_GETN(3, get_rot, getRot);

  static int set_pos(lua_State *L)
  {
    get_ptr(L,1)->setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
    return 0;
  }


  static int set_rot(lua_State *L)
  {
    btMatrix3x3 m;
    m.setEulerYPR(LARG_f(2), LARG_f(3), LARG_f(4));
    get_ptr(L,1)->setRot( m );
    return 0;
  }

  static int add_to_world(lua_State *L)
  {
    if( !physics )
      return luaL_error(L, "physics is not created, no world to add");
    get_ptr(L,1)->addToWorld(physics);
    return 0;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(get_pos);
    LUA_CLASS_MEMBER(get_rot);
    LUA_CLASS_MEMBER(set_pos);
    LUA_CLASS_MEMBER(set_rot);
    LUA_CLASS_MEMBER(add_to_world);
  }
};


class LuaOSimple: public LuaClass<OSimple>
{
  static int _ctor(lua_State *L)
  {
    store_ptr(L, new OSimple());
    return 0;
  }

  static int set_shape(lua_State *L)
  {
    btCollisionShape *shape = *(btCollisionShape **)luaL_checkudata(L, 2, LUA_REGISTRY_PREFIX "Shape"); //XXX
    get_ptr(L,1)->setShape(shape);
    return 0;
  }

  LUA_DEFINE_SET1(set_mass, setMass, LARG_f);
  LUA_DEFINE_GET(is_initialized, isInitialized);

  static int set_pos(lua_State *L)
  {
    if( lua_isnone(L, 4) )
      get_ptr(L,1)->setPosAbove( btVector2(LARG_scaled(2), LARG_scaled(3)) );
    else
      get_ptr(L,1)->setPos( btVector3(LARG_scaled(2), LARG_scaled(3), LARG_scaled(4)) );
    return 0;
  }

  static int set_color(lua_State *L)
  {
    Color4 color;
    LuaManager::checkcolor(L, 2, color);
    get_ptr(L,1)->setColor( color );
    return 0;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(set_shape);
    LUA_CLASS_MEMBER(set_mass);
    LUA_CLASS_MEMBER(is_initialized);
    LUA_CLASS_MEMBER(set_pos);
    LUA_CLASS_MEMBER(set_color);
  }
};


class LuaOGround: public LuaClass<OGround>
{
  static int _ctor(lua_State *L)
  {
    Color4 color, color_t1, color_t2;
    LuaManager::checkcolor(L, 2, color);
    LuaManager::checkcolor(L, 3, color_t1);
    LuaManager::checkcolor(L, 4, color_t2);
    store_ptr(L, new OGround(color, color_t1, color_t2));
    return 0;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
  }
};


LUA_REGISTER_BASE_CLASS(Object);
LUA_REGISTER_SUB_CLASS(OSimple,Object);
LUA_REGISTER_SUB_CLASS(OGround,OSimple);

