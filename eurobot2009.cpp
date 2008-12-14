#include "eurobot2009.h"
#include "colors.h"
#include "object.h"
#include "global.h"
#include "maths.h"


namespace eurobot2009
{
  const dReal table_size_x = 3.0;
  const dReal table_size_y = 2.1;
  const dReal wall_width  = 0.022;
  const dReal wall_height = 0.070;

  const dReal ODispenser::radius = 0.040;
  const dReal ODispenser::height = 0.150;

  ODispenser::ODispenser()
  {
    add_geom(dCreateCylinder(0, radius, height));
    init();
    set_color((Color4)COLOR_PLEXI);
    set_collide(CAT_DYNAMIC);
    add_category(CAT_HANDLER);
  }

  void ODispenser::set_pos(dReal x, dReal y, dReal z, int side)
  {
    switch( side )
    {
      case 0: y -= radius; break;
      case 1: x -= radius; break;
      case 2: y += radius; break;
      case 3: x += radius; break;
      default:
        throw(Error("invalid value for dispenser side"));
    }
    z += height/2;
    //XXX gcc does not find the matching method by itself :(
    ObjectColor::set_pos(x, y, z);
  }

  void ODispenser::fill(Object *o, dReal z)
  {
    const dReal *pos = get_pos();
    o->set_pos(pos[0], pos[1], z);
  }

  void ODispenser::draw()
  {
    glColor4fv(color);
    glPushMatrix();
    draw_move();
    glTranslatef(0, 0, -height/2);
    glutWireCylinder(radius, height, cfg->draw_div, 10);
    glPopMatrix();
  }

  bool ODispenser::collision_handler(Physics *physics, dGeomID o1, dGeomID o2)
  {
    // Only process elements
    if( (dGeomGetCategoryBits(o2) & CAT_ELEMENT) != CAT_ELEMENT )
      return false;

    // Element center not on the dispenser axis: normal processing
    const dReal *pos1 = dGeomGetPosition(o1);
    const dReal *pos2 = dGeomGetPosition(o2);
    if( dist2d(pos1[0],pos1[1], pos2[0],pos2[1]) > radius )
      return false;

    // Element over or under the dispenser: nothing to do
    if( pos2[2] > pos1[2]+height/2 || pos2[2] < pos1[2]-height/2 )
      return true; // nothing to do, collision is processed

    dJointID c = dJointCreateSlider(physics->get_world(), physics->get_joints());
    dJointAttach(c, 0, dGeomGetBody(o2));
    dJointSetSliderAxis(c, 0.0, 0.0, 1.0);
    return true;
  }


  OLintelStorage::OLintelStorage()
  {
    dGeomID geom;

    // Bottom
    geom = dCreateBox(0, 0.200, wall_width, 0.070);
    dGeomSetPosition(geom, 0, -(0.070-3*wall_width)/2, -(0.070-wall_width)/2);
    add_geom(geom);
    // Back
    geom = dCreateBox(0, 0.200, wall_width, .060);
    dGeomSetPosition(geom, 0, (0.070+wall_width)/2, (0.060-wall_width)/2);
    add_geom(geom);
    // Left
    geom = dCreateBox(0, wall_width, 0.070, wall_width);
    dGeomSetPosition(geom, +(0.200-wall_width)/2, 0, 0);
    add_geom(geom);
    // Right
    geom = dCreateBox(0, wall_width, 0.070, wall_width);
    dGeomSetPosition(geom, -(0.200-wall_width)/2, 0, 0);
    add_geom(geom);

    init();

    set_color((Color4)COLOR_BLACK);
  }

  void OLintelStorage::set_pos(dReal d, int side)
  {
    dReal x, y;
    switch( side )
    {
      case 0: x = d; y =  table_size_y/2+0.070/2; break;
      case 1: y = d; x =  table_size_x/2+0.070/2; break;
      case 2: x = d; y = -table_size_y/2-0.070/2; break;
      case 3: y = d; x = -table_size_x/2-0.070/2; break;
      default:
        throw(Error("invalid value for lintel storage side"));
    }

    //XXX gcc does not find the matching method by itself :(
    ObjectColor::set_pos(x, y, wall_height+wall_width/2);
  }

  void OLintelStorage::fill(OLintel *o)
  {
    const dReal *pos = get_pos();
    o->set_pos(pos[0], pos[1], pos[2]+0.030/2+wall_width/2+cfg->drop_epsilon);
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
        get_ptr(L)->ObjectColor::set_pos(LARG_f(2), LARG_f(3));
      else if( lua_isnone(L, 5) )
        get_ptr(L)->ObjectColor::set_pos(LARG_f(2), LARG_f(3), LARG_f(4));
      else
        get_ptr(L)->set_pos(LARG_f(2), LARG_f(3), LARG_f(4), LARG_i(5));
      return 0;
    }

    static int fill(lua_State *L)
    {
      lua_getfield(L, 2, "_ud");
      //XXX check validity
      Object *o = *(Object**)lua_touserdata(L, -1);
      lua_pop(L, 1);
      get_ptr(L)->fill(o, LARG_f(3));
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
        get_ptr(L)->set_pos(LARG_f(2), LARG_i(3));
      else
        get_ptr(L)->ObjectColor::set_pos(LARG_f(2), LARG_f(3), LARG_f(4));
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

