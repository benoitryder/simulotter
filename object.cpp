#include <SDL/SDL.h>
#include <ode/ode.h>

#include "global.h"
#include "object.h"
#include "maths.h"


Object::Object()
{
  LOG->trace("Object: NEW (empty)");
}

Object::Object(dGeomID *geoms, int nb, dBodyID body)
{
  LOG->trace("Object: NEW (multi)");
  ctor_init(geoms, nb, body);
}

Object::Object(dGeomID geom, dBodyID body)
{
  LOG->trace("Object: NEW (single)");
  ctor_init(&geom, 1, body);
}

Object::Object(dGeomID *geoms, int nb, dReal m)
{
  LOG->trace("Object: NEW (multi, mass)");
  ctor_init(geoms, nb, m);
}

Object::Object(dGeomID geom, dReal m)
{
  LOG->trace("Object: NEW (single, mass)");
  ctor_init(&geom, 1, m);
}

void Object::ctor_init(dGeomID *geoms, int nb, dBodyID body)
{
  if( world_void == NULL )
    world_void = dWorldCreate();

  if( body == NULL )
    this->body = dBodyCreate(world_void);
  else
    this->body = body;

  for( int i=0; i<nb; i++ )
  {
    // Position changed after setting body
    // We have to copy position
    const dReal *volatile posv = dGeomGetPosition(geoms[i]);
    dReal pos[3] = { posv[0], posv[1], posv[2] };

    dQuaternion q;
    dGeomGetQuaternion(geoms[i], q);

    dSpaceAdd(physics->get_space(), geoms[i]);
    dGeomSetBody(geoms[i], this->body);
    dGeomSetOffsetPosition(geoms[i], pos[0], pos[1], pos[2]);
    dGeomSetOffsetQuaternion(geoms[i], q);
  }

  this->visible = true;

  this->set_category( body==NULL ? 0 : CAT_DYNAMIC );
  physics->get_objs().push_back(this);
}

void Object::ctor_init(dGeomID *geoms, int nb, dReal m)
{
  ctor_init(geoms, nb, dBodyCreate(physics->get_world()));

  dMass mass;
  if( nb == 1 )
    switch( dGeomGetClass(*geoms) )
    {
      case dSphereClass:
        {
          dReal r = dGeomSphereGetRadius(*geoms);
          dMassSetSphereTotal(&mass, m, r);
          break;
        }
      case dBoxClass:
        {
          dVector3 size;
          dGeomBoxGetLengths(*geoms, size);
          dMassSetBoxTotal(&mass, m, size[0], size[1], size[2]);
          break;
        }
      case dCapsuleClass:
        {
          dReal r, len;
          dGeomCapsuleGetParams(*geoms, &r, &len);
          dMassSetCapsuleTotal(&mass, m, 3, r, len);
          break;
        }
      case dCylinderClass:
        {
          dReal r, len;
          dGeomCylinderGetParams(*geoms, &r, &len);
          dMassSetCylinderTotal(&mass, m, 3, r, len);
          break;
        }
      default:
        {
          dReal a[6];
          this->get_aabb(a);
          dMassSetBoxTotal(&mass, m, a[1]-a[0], a[3]-a[2], a[5]-a[4]);
          break;
        }
    }
  else
  {
    dReal a[6];
    this->get_aabb(a);
    dMassSetBoxTotal(&mass, m, a[1]-a[0], a[3]-a[2], a[5]-a[4]);
  }

  dBodySetMass(this->body, &mass);
}
 

Object::~Object()
{
  OBJECT_FOREACH_GEOM(body,g,dGeomDestroy(g));
  dBodyDestroy(body);
}

dWorldID Object::world_void;


void Object::get_aabb(dReal aabb[6])
{
  dReal a[6];
  dGeomID g = dBodyGetFirstGeom(body);
  dGeomGetAABB(g, aabb);
  for( g=dBodyGetNextGeom(g); g!=NULL; g=dBodyGetNextGeom(g) )
  {
    aabb[0] = MIN(aabb[0],a[0]);
    aabb[1] = MAX(aabb[1],a[1]);
    aabb[2] = MIN(aabb[2],a[2]);
    aabb[3] = MAX(aabb[3],a[3]);
    aabb[4] = MIN(aabb[4],a[4]);
    aabb[5] = MAX(aabb[5],a[5]);
  }
}

void Object::set_pos(dReal x, dReal y)
{
  dReal a[6];
  get_aabb(a);
  set_pos(x, y, a[5]-a[4] + cfg->drop_epsilon);
}

void Object::draw()
{
  OBJECT_FOREACH_GEOM(body,g,draw_geom(g));
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

unsigned long Object::get_category() const
{
  unsigned long cat = 0;
  OBJECT_FOREACH_GEOM(body,g,cat|=dGeomGetCategoryBits(g));
  return cat;
}
unsigned long Object::get_collide() const
{
  unsigned long col = 0;
  OBJECT_FOREACH_GEOM(body,g,col|=dGeomGetCollideBits(g));
  return col;
}

void Object::set_category(unsigned long cat)
{
  OBJECT_FOREACH_GEOM(body,g,dGeomSetCategoryBits(g,cat));
}
void Object::set_collide(unsigned long col)
{
  OBJECT_FOREACH_GEOM(body,g,dGeomSetCollideBits(g,col));
}



const dReal OGround::size_z;
const dReal OGround::size_start;

OGround::OGround(const Color4 color, const Color4 color_t1, const Color4 color_t2):
  Object(dCreateBox(0, Rules::table_size_x, Rules::table_size_y, size_z))
{
  this->geom_box = dBodyGetFirstGeom(this->body);
  dBodySetPosition(this->body, 0, 0, -size_z/2);

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

  set_category(CAT_GROUND);
  set_collide(CAT_DYNAMIC);
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

    // Geoms: table or single Geom
    dGeomID *geoms;
    int nb = 0;

    if( lua_type(L, 2) == LUA_TTABLE )
      geoms = LuaManager::checkudtable<dGeomID>(L, 2, "Geom", &nb);
    else
    {
      nb = 1;
      // Use a temporary variable to use checkudata without memory leak
      dGeomID *ud = (dGeomID*)luaL_checkudata(L, 2, "Geom");
      geoms = new dGeomID[nb];
      geoms[0] = *ud;
    }

    for( int i=0; i<nb; i++ )
      geoms[i] = Physics::geom_duplicate(geoms[i]);

    if( lua_isnoneornil(L, 3) )
      *ud = new Object(geoms, nb);
    else
      *ud = new Object(geoms, nb, LARG_f(3));

    delete[] geoms;
    return 0;
  }

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

    // Geoms: table or single Geom
    dGeomID *geoms;
    int nb = 0;

    if( lua_type(L, 2) == LUA_TTABLE )
      geoms = LuaManager::checkudtable<dGeomID>(L, 2, "Geom", &nb);
    else
    {
      nb = 1;
      // Use a temporary variable to use checkudata without memory leak
      dGeomID *ud = (dGeomID*)luaL_checkudata(L, 2, "Geom");
      geoms = new dGeomID[nb];
      geoms[0] = *ud;
    }

    for( int i=0; i<nb; i++ )
      geoms[i] = Physics::geom_duplicate(geoms[i]);

    if( lua_isnoneornil(L, 3) )
      *ud = new ObjectColor(geoms, nb);
    else
      *ud = new ObjectColor(geoms, nb, LARG_f(3));

    delete[] geoms;
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

