#include "rules2009.h"
#include "colors.h"
#include "object.h"
#include "global.h"


namespace rules2009
{

  RAtlantis::RAtlantis(): Rules((Color4[2]){COLOR_RAL_6018, COLOR_RAL_3020})
  {
  }

  RAtlantis::~RAtlantis() {}


  int RAtlantis::get_score(int team)
  {
    //TODO
    return 0;
  }


  void RAtlantis::init(int fconf)
  {
    int i, j;
    ObjectColor *o;
    ODispenser *od;

    if( fconf == -1 )
      fconf = create_conf(rand()%10, rand()%2);

    int conf_col  = fconf&0xf;
    int conf_disp = fconf>>4;
    LOG->trace("init Atlantis rules, columns: %d, dispensers: %d", conf_col, conf_disp);

    // Ground
    new OGround((Color4)COLOR_RAL_5015, colors[0], colors[1]);

    // Walls (N, E, W, small SE, small SW, plexi S)

    o = new ObjectColor(dCreateBox(0, table_size_x+2*wall_width, wall_width, wall_height));
    o->set_pos(0, +table_size_y/2+wall_width/2, wall_height/2);
    o->set_color((Color4)COLOR_WHITE);
    o = new ObjectColor(dCreateBox(0, wall_width, table_size_y+2*wall_width, wall_height));
    o->set_pos(+table_size_x/2+wall_width/2, 0, wall_height/2);
    o->set_color((Color4)COLOR_WHITE);
    o = new ObjectColor(dCreateBox(0, wall_width, table_size_y+2*wall_width, wall_height));
    o->set_pos(-table_size_x/2-wall_width/2, 0, wall_height/2);
    o->set_color((Color4)COLOR_WHITE);

    o = new ObjectColor(dCreateBox(0, wall_width, 0.100, wall_height));
    o->set_pos(+0.900+wall_width/2, -table_size_y/2+0.050, wall_height/2);
    o->set_color((Color4)COLOR_WHITE);
    o = new ObjectColor(dCreateBox(0, wall_width, 0.100, wall_height));
    o->set_pos(-0.900-wall_width/2, -table_size_y/2+0.050, wall_height/2);
    o->set_color((Color4)COLOR_WHITE);

    o = new ObjectColor(dCreateBox(0, 1.800+wall_width, cfg->draw_epsilon, 0.250));
    o->set_pos(0, -table_size_y/2, 0.125);
    o->set_color((Color4)COLOR_PLEXI);
    o = new ObjectColor(dCreateBox(0, 0.578+wall_width, cfg->draw_epsilon, wall_height));
    o->set_pos(+1.200, -table_size_y/2, wall_height/2);
    o->set_color((Color4)COLOR_PLEXI);
    o = new ObjectColor(dCreateBox(0, 0.578+wall_width, cfg->draw_epsilon, wall_height));
    o->set_pos(-1.200, -table_size_y/2, wall_height/2);
    o->set_color((Color4)COLOR_PLEXI);


    // Building areas
    
    o = new ObjectColor(dCreateBox(0, 1.800, 0.100, 2*cfg->draw_epsilon));
    o->set_pos(0, 0.050-table_size_y/2, cfg->draw_epsilon);
    o->set_color((Color4)COLOR_RAL_8017);
    o->set_category(0);
    o->set_collide(0);

    o = new ObjectColor(dCreateBox(0, 0.600, 0.100, 0.030));
    o->set_pos(0, 0.050-table_size_y/2, 0.015);
    o->set_color((Color4)COLOR_RAL_8017);
    o->set_category(0);
    o->set_collide(CAT_DYNAMIC);

    o = new ObjectColor(dCreateCylinder(0, 0.150, 0.060));
    o->set_pos(0, 0, 0.030);
    o->set_color((Color4)COLOR_RAL_8017);
    o->set_category(0);
    o->set_collide(CAT_DYNAMIC);


    // Random column elements
    for( i=0; i<3; i++ )
    {
      j = col_placements[conf_col][i];

      // First team
      o = new OColElem(); o->set_color(colors[0]);
      o->set_pos(-col_offset_x-(2-(int)(j%3))*col_space_x, -col_offset_y+(3-(int)(j/3))*col_space_y);
      o = new OColElem(); o->set_color(colors[0]);
      o->set_pos(-col_offset_x-(2-(int)(j%3))*col_space_x, -col_offset_y+((int)(j/3))*col_space_y);
      // Second team
      o = new OColElem(); o->set_color(colors[1]);
      o->set_pos(+col_offset_x+(2-(int)(j%3))*col_space_x, -col_offset_y+(3-(int)(j/3))*col_space_y);
      o = new OColElem(); o->set_color(colors[1]);
      o->set_pos(+col_offset_x+(2-(int)(j%3))*col_space_x, -col_offset_y+((int)(j/3))*col_space_y);
    }

    // Fixed dispensers
    od = new ODispenser();
    od->set_pos(table_size_x/2-disp_offset_x, -table_size_y/2, disp_offset_z, 2);
    for( i=1; i<=2; i++ )
    {
      o = new OColElem();
      o->set_color(colors[0]);
      od->fill(o, i*0.035);
    }

    od = new ODispenser();
    od->set_pos(disp_offset_x-table_size_x/2, -table_size_y/2, disp_offset_z, 2);
    for( i=1; i<=2; i++ )
    {
      o = new OColElem();
      o->set_color(colors[1]);
      od->fill(o, i*0.035);
    }

    // Random dispensers
    od = new ODispenser();
    od->set_pos(table_size_x/2-wall_width/2, conf_disp==0 ? disp_offset_y : -disp_offset_y, disp_offset_z, 1);
    for( i=1; i<=2; i++ )
    {
      o = new OColElem();
      o->set_color(colors[0]);
      od->fill(o, i*0.035);
    }

    od = new ODispenser();
    od->set_pos(-table_size_x/2+wall_width/2, conf_disp==0 ? disp_offset_y : -disp_offset_y, disp_offset_z, 3);
    for( i=1; i<=2; i++ )
    {
      o = new OColElem();
      o->set_color(colors[1]);
      od->fill(o, i*0.035);
    }


    // Lintel and lintel storages

    OLintelStorage *ols;
    OLintel *ol;

    ols = new OLintelStorage();
    ols->set_pos(-0.200, 0);
    ol = new OLintel(); ol->set_color(colors[0]);
    ols->fill(ol);

    ols = new OLintelStorage();
    ols->set_pos(-0.600, 0);
    ol = new OLintel(); ol->set_color(colors[0]);
    ols->fill(ol);

    ols = new OLintelStorage();
    ols->set_pos(+0.200, 0);
    ol = new OLintel(); ol->set_color(colors[1]);
    ols->fill(ol);

    ols = new OLintelStorage();
    ols->set_pos(+0.600, 0);
    ol = new OLintel(); ol->set_color(colors[1]);
    ols->fill(ol);
  }

  const int RAtlantis::col_placements[10][3] = {
    {0,1,2}, {0,2,5}, {0,2,4}, {0,2,3}, {0,1,4},
    {0,1,5}, {0,4,5}, {0,1,3}, {0,3,5}, {0,3,4}
  };

  const dReal RAtlantis::col_space_x = 0.250;
  const dReal RAtlantis::col_space_y = 0.200;
  const dReal RAtlantis::col_offset_x = 0.400;
  const dReal RAtlantis::col_offset_y = 0.125;

  const dReal RAtlantis::disp_offset_x = 0.289;
  const dReal RAtlantis::disp_offset_y = 0.250;
  const dReal RAtlantis::disp_offset_z = 0.045;

  const dReal RAtlantis::wall_width  = 0.022;
  const dReal RAtlantis::wall_height = 0.070;


  const dReal ODispenser::radius = 0.040;
  const dReal ODispenser::height = 0.150;

  ODispenser::ODispenser():
    ObjectColor(dCreateCylinder(0, radius, height))
  {
    set_color((Color4)COLOR_PLEXI);
    set_category(CAT_DISPENSER);
    set_collide(CAT_DYNAMIC);
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
    glColor3fv(color);
    glPushMatrix();
    draw_move();
    glTranslatef(0, 0, -height/2);
    glutWireCylinder(radius, height, cfg->draw_div, 10);
    glPopMatrix();
  }


  OLintelStorage::OLintelStorage():
    ObjectColor()
  {
    dGeomID geoms[4];

    // Bottom
    geoms[0] = dCreateBox(0, 0.200, RAtlantis::wall_width, 0.070);
    dGeomSetPosition(geoms[0], 0, -(0.070-3*RAtlantis::wall_width)/2, -(0.070-RAtlantis::wall_width)/2);
    // Back
    geoms[1] = dCreateBox(0, 0.200, RAtlantis::wall_width, .060);
    dGeomSetPosition(geoms[1], 0, (0.070+RAtlantis::wall_width)/2, (0.060-RAtlantis::wall_width)/2);
    // Left
    geoms[2] = dCreateBox(0, RAtlantis::wall_width, 0.070, RAtlantis::wall_width);
    dGeomSetPosition(geoms[2], +(0.200-RAtlantis::wall_width)/2, 0, 0);
    // Right
    geoms[3] = dCreateBox(0, RAtlantis::wall_width, 0.070, RAtlantis::wall_width);
    dGeomSetPosition(geoms[3], -(0.200-RAtlantis::wall_width)/2, 0, 0);

    ctor_init(geoms, 4, (dBodyID)NULL);

    set_collide(CAT_DYNAMIC);

    set_color((Color4)COLOR_BLACK);
  }

  void OLintelStorage::set_pos(dReal d, int side)
  {
    dReal x, y;
    switch( side )
    {
      case 0: x = d; y =  RAtlantis::table_size_y/2+0.070/2; break;
      case 1: y = d; x =  RAtlantis::table_size_x/2+0.070/2; break;
      case 2: x = d; y = -RAtlantis::table_size_y/2-0.070/2; break;
      case 3: y = d; x = -RAtlantis::table_size_x/2-0.070/2; break;
      default:
        throw(Error("invalid value for lintel storage side"));
    }

    //XXX gcc does not find the matching method by itself :(
    ObjectColor::set_pos(x, y, RAtlantis::wall_height+RAtlantis::wall_width/2);
  }

  void OLintelStorage::fill(OLintel *o)
  {
    const dReal *pos = get_pos();
    o->set_pos(pos[0], pos[1], pos[2]+0.030/2+RAtlantis::wall_width/2+cfg->drop_epsilon);
  }
}

