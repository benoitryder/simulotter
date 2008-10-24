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
    ObjectDynamicColor *o;
    ObjectColor *so;

    if( fconf == -1 )
      fconf = create_conf(rand()%10, rand()%2);

    int conf_col  = fconf&0xf;
    int conf_disp = fconf>>4;
    LOG->trace("init Atlantis rules, columns: %d, dispensers: %d", conf_col, conf_disp);

    // Ground
    new OGround((Color4)COLOR_RAL_5015, colors[0], colors[1]);

    // Building areas
    
    so = new ObjectColor(dCreateBox(0, 1.800, 0.100, 2*cfg->draw_epsilon));
    so->set_pos(0, 0.050-table_size_y/2, cfg->draw_epsilon);
    so->set_color((Color4)COLOR_RAL_8017);
    so->set_category(0);
    so->set_collide(0);

    so = new ObjectColor(dCreateBox(0, 0.600, 0.100, 0.030));
    so->set_pos(0, 0.050-table_size_y/2, 0.015);
    so->set_color((Color4)COLOR_RAL_8017);
    so->set_category(0);
    so->set_collide(CAT_DYNAMIC);

    so = new ObjectColor(dCreateCylinder(0, 0.150, 0.060));
    so->set_pos(0, 0, 0.030);
    so->set_color((Color4)COLOR_RAL_8017);
    so->set_category(0);
    so->set_collide(CAT_DYNAMIC);


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

    ODispenser *od;

    // Fixed dispensers
    od = new ODispenser();
    od->set_pos(table_size_x/2-disp_offset_x, -table_size_y/2, disp_offset_z, 2);
    od = new ODispenser();
    od->set_pos(disp_offset_x-table_size_x/2, -table_size_y/2, disp_offset_z, 2);

    // Random dispensers
    od = new ODispenser();
    od->set_pos(table_size_x/2, conf_disp==0 ? disp_offset_y : -disp_offset_y, disp_offset_z, 1);
    od = new ODispenser();
    od->set_pos(-table_size_x/2, conf_disp==0 ? disp_offset_y : -disp_offset_y, disp_offset_z, 2);


    //o = new OLintel(); o->set_color(colors[1]);
    //o->set_pos(0,0);
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


  ODispenser::ODispenser():
    ObjectColor(dCreateCylinder(0, 0.040, 0.150))
  {
    set_color((Color4)COLOR_GRAY(0.9));
    set_category(CAT_DISPENSER);
    set_collide(CAT_DYNAMIC);
  }

  void ODispenser::set_pos(dReal x, dReal y, dReal z, int side)
  {
    dReal r, l;
    dGeomCylinderGetParams(geom, &r, &l);
    switch( side )
    {
      case 0: y -= r; break;
      case 1: x -= r; break;
      case 2: y += r; break;
      case 3: x += r; break;
      default:
        throw(Error("invalid value for dispenser side"));
    }
    z += l/2;
    dGeomSetPosition(geom, x, y, z);
  }

  void ODispenser::draw()
  {
    glColor3fv(color);
    glPushMatrix();
    draw_move(geom);
    dReal r, len;
    dGeomCylinderGetParams(geom, &r, &len);
    glTranslatef(0, 0, -len/2);
    glutWireCylinder(r, len, cfg->draw_div, 10);
    glPopMatrix();
  }


  OLintelStorage::OLintelStorage():
    ObjectColor(dCreateCylinder(0, 0.040, 0.150))
  {
    set_category(CAT_DISPENSER);
    set_collide(CAT_DYNAMIC);
  }
}

