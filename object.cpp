#include <SDL/SDL.h>
#include <ode/ode.h>

#include "global.h"
#include "object.h"
#include "maths.h"


Object::Object()
{
  LOG->trace("Object: NEW");
  body = NULL;
  init_mass = 0;
  initialized = false;
  visible = true;
}

void Object::add_geom(dGeomID geom)
{
  if( is_initialized() )
    throw Error("add_geom(): attempt to modify an initialized object");
  this->geoms.push_back(geom);
}

void Object::set_body(dBodyID body)
{
  if( body == NULL )
    return;
  if( this->init_mass != 0 )
    throw Error("set_body(): cannot set both mass and body");
  if( is_initialized() )
    throw Error("set_body(): attempt to modify an initialized object");
  this->body = body;
}

void Object::set_mass(dReal m)
{
  if( m <= 0 )
    return;
  if( this->body != NULL )
    throw Error("set_mass(): cannot set both body and mass");
  if( is_initialized() )
    throw Error("set_mass(): attempt to modify an initialized object");
  this->init_mass = m;
}

void Object::init()
{
  if( is_initialized() )
    throw Error("object is already initialized");

  bool is_static = ( body == NULL && init_mass == 0 );

  std::vector<dGeomID>::iterator it;

  if( body == NULL )
    this->body = dBodyCreate(physics->get_world());

  // Set mass from geoms.
  // Masses are created for each geom, with the same density, and added.
  // Then, total mass is adjusted.
  if( init_mass != 0 )
  {
    dMass body_mass, geom_mass;
    dMassSetZero(&body_mass);

    for( it=geoms.begin(); it!=geoms.end(); ++it )
    {
      switch( dGeomGetClass(*it) )
      {
        case dSphereClass:
          {
            dReal r = dGeomSphereGetRadius(*it);
            dMassSetSphere(&geom_mass, 1.0, r);
            break;
          }
        case dBoxClass:
          {
            dVector3 size;
            dGeomBoxGetLengths(*it, size);
            dMassSetBox(&geom_mass, 1.0, size[0], size[1], size[2]);
            break;
          }
        case dCapsuleClass:
          {
            dReal r, len;
            dGeomCapsuleGetParams(*it, &r, &len);
            dMassSetCapsule(&geom_mass, 1.0, 3, r, len);
            break;
          }
        case dCylinderClass:
          {
            dReal r, len;
            dGeomCylinderGetParams(*it, &r, &len);
            dMassSetCylinder(&geom_mass, 1.0, 3, r, len);
            break;
          }
        default:
          throw Error("geom class not supported for object mass");
      }

      const dReal *pos = dGeomGetPosition(*it);
      const dReal *rot = dGeomGetRotation(*it);
      dMassTranslate(&geom_mass, pos[0], pos[1], pos[2]);
      dMassRotate(&geom_mass, rot);

      dMassAdd(&body_mass, &geom_mass);
    }

    dMassAdjust(&body_mass, this->init_mass);
    dBodySetMass(this->body, &body_mass);
  }

  // Geoms
  for( it=geoms.begin(); it!=geoms.end(); ++it )
  {
    // Position changed after setting body
    // We have to copy position
    const dReal *volatile posv = dGeomGetPosition(*it);
    dReal pos[3] = { posv[0], posv[1], posv[2] };

    dQuaternion q;
    dGeomGetQuaternion(*it, q);

    dSpaceAdd(physics->get_space(), *it);
    dGeomSetBody(*it, this->body);
    dGeomSetOffsetPosition(*it, pos[0], pos[1], pos[2]);
    dGeomSetOffsetQuaternion(*it, q);
  }

  if( is_static )
  {
    dBodyDisable(this->body);
    dBodySetGravityMode(this->body, 0);
    set_category(CAT_NONE);
    set_collide(CAT_DYNAMIC);
  }
  else
  {
    set_category(CAT_DYNAMIC);
    set_collide(CAT_ALL);
  }

  physics->get_objs().push_back(this);

  this->initialized = true;
}



Object::~Object()
{
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    dGeomDestroy(*it);
  dBodyDestroy(body);
}


void Object::get_aabb(dReal aabb[6])
{
  dReal a[6];

  std::vector<dGeomID>::iterator it = geoms.begin();
  dGeomGetAABB(*it, aabb);
  for( ++it; it!=geoms.end(); ++it )
  {
    dGeomGetAABB(*it, a);
    aabb[0] = MIN(aabb[0],a[0]);
    aabb[1] = MAX(aabb[1],a[1]);
    aabb[2] = MIN(aabb[2],a[2]);
    aabb[3] = MAX(aabb[3],a[3]);
    aabb[4] = MIN(aabb[4],a[4]);
    aabb[5] = MAX(aabb[5],a[5]);
  }
}

void Object::set_pos(dReal x, dReal y, dReal z)
{
  if( !is_initialized() )
    throw Error("set_pos(): object is not initialized");
  dBodySetPosition(body, x, y, z);
}

void Object::set_pos(dReal x, dReal y)
{
  if( !is_initialized() )
    throw Error("set_pos(): object is not initialized");
  dReal a[6];
  get_aabb(a);
  set_pos(x, y, a[5]-a[4] + cfg->drop_epsilon);
}

void Object::set_rot(const dQuaternion q)
{
  if( !is_initialized() )
    throw Error("set_rot(): object is not initialized");
  dBodySetQuaternion(body, q);
}

void Object::draw()
{
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    draw_geom(*it);
}

void Object::draw_geom(dGeomID geom)
{
  glPushMatrix();

  draw_move(geom);

  switch( dGeomGetClass(geom) )
  {
    case dSphereClass:
      glutSolidSphere(dGeomSphereGetRadius(geom), cfg->draw_div, cfg->draw_div);
      break;
    case dBoxClass:
      {
        dVector3 size;
        dGeomBoxGetLengths(geom, size);
        glScalef(size[0], size[1], size[2]);
        glutSolidCube(1.0f);
        break;
      }
    case dCapsuleClass:
      {
        dReal r, len;
        dGeomCapsuleGetParams(geom, &r, &len);
        glTranslatef(0, 0, -len/2);
        glutSolidCylinder(r, len, cfg->draw_div, cfg->draw_div);
        glutSolidSphere(r, cfg->draw_div, cfg->draw_div);
        glTranslatef(0, 0, len);
        glutSolidSphere(r, cfg->draw_div, cfg->draw_div);
        break;
      }
    case dCylinderClass:
      {
        dReal r, len;
        dGeomCylinderGetParams(geom, &r, &len);
        glTranslatef(0, 0, -len/2);
        glutSolidCylinder(r, len, cfg->draw_div, cfg->draw_div);
        break;
      }
    default:
      throw(Error("drawing not supported for this geometry class"));
      break;
  }

  glPopMatrix();
}


void Object::draw_move(dGeomID geom)
{
  const dReal *pos = dGeomGetPosition(geom);
  const dReal *rot = dGeomGetRotation(geom);

  GLfloat m[16] = {
    rot[0], rot[4], rot[8],  0.0f,
    rot[1], rot[5], rot[9],  0.0f,
    rot[2], rot[6], rot[10], 0.0f,
    pos[0], pos[1], pos[2],  1.0f
  };
  glMultMatrixf(m);
}

void Object::draw_move()
{
  const dReal *pos = dBodyGetPosition(body);
  const dReal *rot = dBodyGetRotation(body);

  GLfloat m[16] = {
    rot[0], rot[4], rot[8],  0.0f,
    rot[1], rot[5], rot[9],  0.0f,
    rot[2], rot[6], rot[10], 0.0f,
    pos[0], pos[1], pos[2],  1.0f
  };
  glMultMatrixf(m);
}

unsigned long Object::get_category()
{
  unsigned long cat = 0;
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    cat |= dGeomGetCategoryBits(*it);
  return cat;
}
unsigned long Object::get_collide()
{
  unsigned long col = 0;
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    col |= dGeomGetCollideBits(*it);
  return col;
}

void Object::set_category(unsigned long cat)
{
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    dGeomSetCategoryBits(*it, cat);
}
void Object::set_collide(unsigned long col)
{
  std::vector<dGeomID>::iterator it;
  for( it=geoms.begin(); it!=geoms.end(); ++it )
    dGeomSetCollideBits(*it, col);
}



const dReal OGround::size_z;
const dReal OGround::size_start;

OGround::OGround(const Color4 color, const Color4 color_t1, const Color4 color_t2)
{
  this->geom_box = dCreateBox(0, Rules::table_size_x, Rules::table_size_y, size_z);
  add_geom(this->geom_box);
  init();

  set_category(CAT_GROUND);
  set_pos(0, 0, -size_z/2);

  this->color[0] = color[0];
  this->color[1] = color[1];
  this->color[2] = color[2];
  this->color[3] = 0.0;

  this->color_t1[0] = color_t1[0];
  this->color_t1[1] = color_t1[1];
  this->color_t1[2] = color_t1[2];
  this->color_t1[3] = 0.0;

  this->color_t2[0] = color_t2[0];
  this->color_t2[1] = color_t2[1];
  this->color_t2[2] = color_t2[2];
  this->color_t2[3] = 0.0;
}

void OGround::draw()
{
  glPushMatrix();

  draw_move();

  // Ground

  glPushMatrix();

  glColor3fv(color);
  dReal size[3];
  dGeomBoxGetLengths(geom_box, size);
  glScalef(size[0], size[1], size[2]);
  glutSolidCube(1.0f);

  glPopMatrix();


  // Starting areas

  glPushMatrix();

  glColor3fv(color_t1);
  glTranslatef(-(size[0]-size_start)/2, (size[1]-size_start)/2, size[2]/2);
  glScalef(size_start, size_start, 2*cfg->draw_epsilon);
  glutSolidCube(1.0f);

  glPopMatrix();

  glPushMatrix();

  glColor3fv(color_t2);
  glTranslatef((size[0]-size_start)/2, (size[1]-size_start)/2, size[2]/2);
  glScalef(size_start, size_start, 2*cfg->draw_epsilon);
  glutSolidCube(1.0f);

  glPopMatrix();

  glPopMatrix();
}



class LuaObject: public LuaClass<Object>
{
  //TODO category/collide

  static int _ctor(lua_State *L)
  {
    Object **ud = new_userdata(L);
    *ud = new Object();
    return 0;
  }

  static int add_geom(lua_State *L)
  {
    dGeomID geom = *(dGeomID*)luaL_checkudata(L, 2, "Geom");
    get_ptr(L)->add_geom( Physics::geom_duplicate(geom) );
    return 0;
  }

  LUA_DEFINE_SET1(set_mass, LARG_f);
  LUA_DEFINE_SET0(init);
  LUA_DEFINE_GET(is_initialized);

  static int get_pos(lua_State *L)
  {
    const dReal *pos = get_ptr(L)->get_pos();
    push(L, pos[0]);
    push(L, pos[1]);
    push(L, pos[2]);
    return 3;
  }

  static int set_pos(lua_State *L)
  {
    if( lua_isnone(L, 4) )
      get_ptr(L)->set_pos(LARG_f(2), LARG_f(3));
    else
      get_ptr(L)->set_pos(LARG_f(2), LARG_f(3), LARG_f(4));
    return 0;
  }

  static int set_rot(lua_State *L)
  {
    dQuaternion q;
    for( int i=0; i<4; i++ )
      q[i] = LARG_f(i+2);
    get_ptr(L)->set_rot(q);
    return 0;
  }

  LUA_DEFINE_SET1(set_visible, LARG_b)

public:
  LuaObject()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(add_geom);
    LUA_REGFUNC(set_mass);
    LUA_REGFUNC(init);
    LUA_REGFUNC(is_initialized);
    LUA_REGFUNC(get_pos);
    LUA_REGFUNC(set_pos);
    LUA_REGFUNC(set_rot);
    LUA_REGFUNC(set_visible);
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
    get_ptr(L)->set_color( color );
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

