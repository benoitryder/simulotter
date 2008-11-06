#include <string.h>
#include <ode/ode.h>
#include "global.h"
#include "robot.h"
#include "maths.h"


Physics::Physics()
{
  dInitODE();
  pause_state = false;

  world = dWorldCreate();
  space = dSimpleSpaceCreate(0);

  dWorldSetGravity(world, 0, 0, cfg->gravity_z);
  dWorldSetCFM(world, cfg->cfm);

  joints = dJointGroupCreate(0);
  dJointGroupEmpty(joints);
}

Physics::~Physics()
{
  std::vector<Object*>::iterator it;
  for( it = objs.begin(); it != objs.end(); ++it )
    delete *it;
  objs.clear();

  dSpaceDestroy(space);
  dWorldDestroy(world);

  dCloseODE();
}


void Physics::step()
{
  if( this->pause_state )
    return;

  std::vector<dGeomID>::iterator it2;
  for( it2=hack_boxes.begin(); it2!=hack_boxes.end(); ++it2 )
    dGeomDestroy(*it2);
  hack_boxes.clear();
  hack_cylinders.clear();

  dSpaceCollide(space, this, &Physics::collide_callback);

  // Cylinder hack
  // Convert the top cylinder into a box
  // Fine with small cylinders (h < r)
  // XXX use two capsules if there are no contact on bases?
  std::vector<GeomPair>::iterator it;
  for( it=hack_cylinders.begin(); it!=hack_cylinders.end(); ++it )
  {
    dGeomID o1 = (*it).o1;
    dGeomID o2 = (*it).o2;
    const dReal *v1, *v2;

    v1 = dGeomGetPosition(o1);
    v2 = dGeomGetPosition(o2);
    // The box should be the top element, swap if needed
    //XXX Use bounding box ?
    //XXX Really better to transform the top element?
    if( v2[2] > v1[2] )
    {
      o1 = (*it).o2;
      o2 = (*it).o1;
      v1 = dGeomGetPosition(o1);
      v2 = dGeomGetPosition(o2);
    }

    dReal r, l;
    dGeomCylinderGetParams(o1, &r, &l);
    dGeomID b1 = dCreateBox(0, 2*r, 2*r, l);
    dGeomSetBody(b1, dGeomGetBody(o1));

    dGeomSetCategoryBits(b1, dGeomGetCategoryBits(o1));

    const dReal *R1 = dGeomGetRotation(o1);
    const dReal *R2 = dGeomGetRotation(o2);

    // vc: vecteur joignant les centres
    dVector3 vc, v, vv;
    vc[0]=v2[0]-v1[0]; vc[1]=v2[1]-v1[1]; vc[2]=v2[2]-v1[2];
    // On garde la composante orthogonale à l'axe de o2
    // On passe dans le repère de o2, on met z à 0 et on rechange de repère
    v[0] = R2[0]*vc[0]+R2[4]*vc[1]+R2[8]*vc[2];
    v[1] = R2[1]*vc[0]+R2[5]*vc[1]+R2[9]*vc[2];
    vv[0] = R2[0]*v[0]+R2[1]*v[1];
    vv[1] = R2[4]*v[0]+R2[5]*v[1];
    vv[2] = R2[8]*v[0]+R2[9]*v[1];

    // Passage dans le repère de o1, on veut v colinéaire à x1
    v[0] = R1[0]*vv[0]+R1[4]*vv[1]+R1[8]*vv[2];
    v[1] = R1[1]*vv[0]+R1[5]*vv[1]+R1[9]*vv[2];

    if( !isnan(v[1]/v[0]) )
    {
      dQuaternion q;
      dQFromAxisAndAngle(q, 0, 0, 1, atan(v[1]/v[0]));
      dGeomSetOffsetQuaternion(b1, q);
    }

    collide_callback(this, b1, o2);
    //Note: uncomment to display hack_boxes
    // AND comment dGeomDestroy();
    //hack_boxes.push_back(b1);
    dGeomDestroy(b1);
  }

  dWorldStep(world, cfg->step_dt);
  dJointGroupEmpty(joints);

  // Update robot values, do asserv and strategy
  std::vector<Robot*> &robots = Robot::get_robots();
  std::vector<Robot*>::iterator itr;
  for( itr=robots.begin(); itr!=robots.end(); ++itr )
  {
    (*itr)->update();
    (*itr)->asserv();
    (*itr)->strategy();
  }
}


dGeomID Physics::geom_duplicate(dGeomID geom)
{
  dGeomID g;
  switch( dGeomGetClass(geom) )
  {
    case dSphereClass:
      {
        dReal r = dGeomSphereGetRadius(geom);
        g = dCreateSphere(0, r);
        break;
      }
    case dBoxClass:
      {
        dVector3 size;
        dGeomBoxGetLengths(geom, size);
        g = dCreateBox(0, size[0], size[1], size[2]);
        break;
      }
    case dPlaneClass:
      {
        dVector4 p;
        dGeomPlaneGetParams(geom, p);
        g = dCreatePlane(0, p[0], p[1], p[2], p[3]);
        break;
      }
    case dCapsuleClass:
      {
        dReal r, len;
        dGeomCapsuleGetParams(geom, &r, &len);
        g = dCreateCapsule(0, r, len);
        break;
      }
    case dCylinderClass:
      {
        dReal r, len;
        dGeomCylinderGetParams(geom, &r, &len);
        g = dCreateCylinder(0, r, len);
        break;
      }
    case dRayClass:
      {
        g = dCreateRay(0, dGeomRayGetLength(geom));
        break;
      }
    default:
      {
        throw(Error("geom class not supported in geom_duplicate"));
        break;
      }
  }

  const dReal *pos = dGeomGetPosition(geom);
  const dReal *rot = dGeomGetRotation(geom);
  dGeomSetPosition(g, pos[0], pos[1], pos[2]);
  dGeomSetRotation(g, rot);
  return g;
}


void Physics::collide_callback(void *data, dGeomID o1, dGeomID o2)
{
  int i, n;
  unsigned long cat1, cat2;
  dBodyID b1, b2;
  dSurfaceParameters sp;

  cat1 = dGeomGetCategoryBits(o1);
  cat2 = dGeomGetCategoryBits(o2);

  b1 = ((cat1 & CAT_DYNAMIC)==0) ? 0 : dGeomGetBody(o1);
  b2 = ((cat2 & CAT_DYNAMIC)==0) ? 0 : dGeomGetBody(o2);

  if( b1 == b2 )
    return; // Geoms of the same object do not collide

  // Ignore elements in dispensers
  //TODO only ignore if completely inside
  if( (cat1==CAT_DISPENSER) && (cat2==CAT_ELEMENT) ||
      (cat2==CAT_DISPENSER) && (cat1==CAT_ELEMENT) )
  {
    dJointID c = dJointCreateSlider(physics->get_world(), physics->get_joints());
    dJointAttach(c, b1, b2);
    dJointSetSliderAxis(c, 0.0, 0.0, 1.0);
    return;
  }

  // Default surface parameters
  sp.mode = dContactSlip1 | dContactSlip2 | dContactApprox1 | dContactBounce;
  sp.mu = dInfinity;
  sp.slip1 = 0.01;
  sp.slip2 = 0.01;
  sp.bounce = 0.05;
  //sp.bounce_vel = 0.010;

  dContact contacts[cfg->contacts_nb];

  // Store them to hack them later
  if( dGeomGetClass(o1)==dCylinderClass && dGeomGetClass(o2)==dCylinderClass )
  {
    ((Physics*)data)->hack_cylinders.push_back((GeomPair){o1,o2});
    return;
  }

  n = dCollide(o1, o2, cfg->contacts_nb, &contacts[0].geom, sizeof(*contacts));

  for( i=0; i<n; i++ )
  {
    /*
    dBodyID b1, b2;
    // Robots are not affected by elements
    if( (cat1&CAT_ROBOT)==CAT_ROBOT && (cat2&CAT_ELEMENT)==CAT_ELEMENT )
      b1 = 0;
    else
      b1 = dGeomGetBody(contacts[i].geom.g1);
    if( (cat2&CAT_ROBOT)==CAT_ROBOT && (cat1&CAT_ELEMENT)==CAT_ELEMENT )
      b2 = 0;
    else
      b2 = dGeomGetBody(contacts[i].geom.g2);
      */
    b1 = ((cat1 & CAT_DYNAMIC)==0) ? 0 : dGeomGetBody(contacts[i].geom.g1);
    b2 = ((cat2 & CAT_DYNAMIC)==0) ? 0 : dGeomGetBody(contacts[i].geom.g2);

    memcpy(&contacts[i].surface, &sp, sizeof(sp));;
    dJointID c = dJointCreateContact(physics->get_world(), physics->get_joints(), &contacts[i]);
    dJointAttach(c, b1, b2);
  }
}


/** @name Lua geoms
 *
 * Lua geoms are not standard class: instances are created using a specific
 * method for each geom each.
 * Instances are not tables with an \e _ud fields be userdata.
 * new_userdata() and get_ptr() are redefined for this purpose.
 *
 * Geoms are duplicated, and thus can be reused and properly collected
 * properly.
 */
//@{

class LuaGeom: public LuaClass<dGeomID>
{
  static dGeomID *new_userdata(lua_State *L)
  {
    // check that we use Class:method and not Class.method
    luaL_checktype(L, 1, LUA_TUSERDATA);
    dGeomID *ud = (dGeomID *)lua_newuserdata(L, sizeof(dGeomID));
    lua_getfield(L, LUA_REGISTRYINDEX, name);
    lua_setmetatable(L, -2);
    return ud;
  }

  static dGeomID get_ptr(lua_State *L)
  {
    return *(dGeomID*)luaL_checkudata(L, 1, name);
  }

  static int _ctor(lua_State *L)
  {
    return luaL_error(L, "no Geom constructor, use creation methods");
  }

  static int sphere(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreateSphere(0, LARG_f(2));
    return 1;
  }

  static int box(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreateBox(0, LARG_f(2), LARG_f(3), LARG_f(4));
    return 1;
  }

  static int plane(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreatePlane(0, LARG_f(2), LARG_f(3), LARG_f(4), LARG_f(5));
    return 1;
  }

  static int capsule(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreateCapsule(0, LARG_f(2), LARG_f(3));
    return 1;
  }

  static int cylinder(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreateCylinder(0, LARG_f(2), LARG_f(3));
    return 1;
  }

  static int ray(lua_State *L)
  {
    dGeomID *ud = new_userdata(L);
    *ud = dCreateCapsule(0, LARG_f(2), LARG_f(3));
    return 1;
  }


  static int set_pos(lua_State *L)
  {
    dGeomID geom = get_ptr(L);
    dGeomSetPosition(geom, LARG_f(2), LARG_f(3), LARG_f(4));
    return 0;
  }

  static int set_rot(lua_State *L)
  {
    dGeomID geom = get_ptr(L);
    const dQuaternion q = { LARG_f(2), LARG_f(3), LARG_f(4), LARG_f(5) };
    dGeomSetQuaternion(geom, q);
    return 0;
  }

  // Garbage collector
  static int gc(lua_State *L)
  {
    dGeomID geom = get_ptr(L);
    // Don't destroy used geoms (should not happened, just in case)
    if( dGeomGetSpace(geom) == 0 )
      dGeomDestroy( geom );
    return 0;
  }

public:
  LuaGeom()
  {
    LUA_REGFUNC(_ctor);
    LUA_REGFUNC(sphere);
    LUA_REGFUNC(box);
    LUA_REGFUNC(plane);
    LUA_REGFUNC(capsule);
    LUA_REGFUNC(cylinder);
    LUA_REGFUNC(ray);
    LUA_REGFUNC(set_pos);
    LUA_REGFUNC(set_rot);
    functions.push_back((LuaRegFunc){"__gc", gc});
  }
};

template<> const char *LuaClass<dGeomID>::name = "Geom";
static LuaGeom register_LuaGeom;
template<> const char *LuaClass<dGeomID>::base_name = NULL;

//@}

