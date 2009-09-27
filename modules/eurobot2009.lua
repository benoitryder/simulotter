
-- Eurobot 2009: Atlantis

require('modules/colors')
module('eurobot2009', package.seeall)

-- Team colors
color1 = colors.ral_6018
color2 = colors.ral_3020

-- Match init
function init(fconf)
  -- Field configuration:
  --   columns: 1 to 10
  --   dispensers: (1 or 2) * 16
  -- Can be defined as 2-digit hexadecimal value: 0x<disp><col>
  local conf_col, conf_disp
  if fconf == nil or fconf < 0 then
    conf_col  = math.random(10)
    conf_disp = math.random(2)
  else
    conf_col  = fconf % 16 + 1
    conf_disp = math.floor(fconf/16) + 1
  end
  if conf_col < 1 or conf_col > 10 or conf_disp < 1 or conf_disp > 2 then
    error("invalid field configuration")
  end
  trace("Atlantis rules, columns: "..tostring(conf_col)..", dispensers: "..tostring(conf_disp))

  -- Various variables

  local table_size_x = 3.0
  local table_size_y = 2.1

  local col_space_x = 0.250
  local col_space_y = 0.200
  local col_offset_x = 0.400
  local col_offset_y = 0.125

  local disp_offset_x = 0.289
  local disp_offset_y = 0.250
  local disp_offset_z = 0.045

  local wall_width  = 0.022
  local wall_height = 0.070
  

  local o, ol, ols


  -- Ground
  trace("  ground")

  o = OGround(colors.ral_5015, color1, color2)
  o:add_to_world()

  -- Walls (N, E, W, small SE, small SW, plexi S)
  trace("  walls")

  o = OSimple()
  o:set_shape(Shape:box(table_size_x/2+wall_width, wall_width/2, wall_height/2))
  o:add_to_world()
  o:set_pos(0, table_size_y/2+wall_width/2, wall_height/2)
  o:set_color(colors.white)
  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, table_size_y/2+wall_width, wall_height/2))
  o:add_to_world()
  o:set_pos(table_size_x/2+wall_width/2, 0, wall_height/2)
  o:set_color(colors.white)
  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, table_size_y/2+wall_width, wall_height/2))
  o:add_to_world()
  o:set_pos(-table_size_x/2-wall_width/2, 0, wall_height/2)
  o:set_color(colors.white)

  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, 0.100/2, wall_height/2))
  o:add_to_world()
  o:set_pos(0.900+wall_width/2, -table_size_y/2+0.050, wall_height/2)
  o:set_color(colors.white)
  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, 0.100/2, wall_height/2))
  o:add_to_world()
  o:set_pos(-0.900-wall_width/2, -table_size_y/2+0.050, wall_height/2)
  o:set_color(colors.white)

  o = OSimple()
  o:set_shape(Shape:box(1.800/2+wall_width/2, config.draw_epsilon, 0.250/2))
  o:add_to_world()
  o:set_pos(0, -table_size_y/2, 0.125)
  o:set_color(colors.plexi)
  o = OSimple()
  o:set_shape(Shape:box(0.578/2+wall_width/2, config.draw_epsilon, wall_height/2))
  o:add_to_world()
  o:set_pos(1.200, -table_size_y/2, wall_height/2)
  o:set_color(colors.plexi)
  o = OSimple()
  o:set_shape(Shape:box(0.578/2+wall_width/2, config.draw_epsilon, wall_height/2))
  o:add_to_world()
  o:set_pos(-1.200, -table_size_y/2, wall_height/2)
  o:set_color(colors.plexi)


  -- Building areas
  trace("  building areas")

  o = OSimple()
  o:set_shape(Shape:box(1.800/2, 0.100/2, config.draw_epsilon))
  o:add_to_world()
  o:set_pos(0, 0.050-table_size_y/2, config.draw_epsilon)
  o:set_color(colors.ral_8017)

  o = OSimple()
  o:set_shape(Shape:box(0.600/2, 0.100/2, 0.030/2))
  o:add_to_world()
  o:set_pos(0, 0.050-table_size_y/2, 0.015)
  o:set_color(colors.ral_8017)

  o = OSimple()
  o:set_shape(Shape:cylinderZ(0.150, 0.060/2))
  o:add_to_world()
  o:set_pos(0, 0, 0.030)
  o:set_color(colors.ral_8017)


  -- Random column elements
  trace("  random columns elements")

  local col_placements = {
    {0,1,2}, {0,2,5}, {0,2,4}, {0,2,3}, {0,1,4},
    {0,1,5}, {0,4,5}, {0,1,3}, {0,3,5}, {0,3,4}
  }

  for i,j in ipairs(col_placements[conf_col]) do
    -- First team
    o = OColElem()
    o:add_to_world()
    o:set_color(color1)
    o:set_pos(-col_offset_x-(2-math.floor(j%3))*col_space_x, -col_offset_y+(3-math.floor(j/3))*col_space_y)
    o = OColElem()
    o:add_to_world()
    o:set_color(color1)
    o:set_pos(-col_offset_x-(2-math.floor(j%3))*col_space_x, -col_offset_y+math.floor(j/3)*col_space_y)
    -- Second team
    o = OColElem()
    o:add_to_world()
    o:set_color(color2)
    o:set_pos(col_offset_x+(2-math.floor(j%3))*col_space_x, -col_offset_y+(3-math.floor(j/3))*col_space_y)
    o = OColElem()
    o:add_to_world()
    o:set_color(color2)
    o:set_pos(col_offset_x+(2-math.floor(j%3))*col_space_x, -col_offset_y+math.floor(j/3)*col_space_y)
  end


  -- Dispensers
  trace("  dispensers")

  -- Fixed
  od = ODispenser()
  od:add_to_world()
  od:set_pos(table_size_x/2-disp_offset_x, -table_size_y/2, disp_offset_z, 2)
  for i = 1,5 do
    o = OColElem()
    o:add_to_world()
    o:set_color(color1)
    od:fill(o, i*0.035)
  end
  od = ODispenser()
  od:add_to_world()
  od:set_pos(disp_offset_x-table_size_x/2, -table_size_y/2, disp_offset_z, 2)
  for i = 1,5 do
    o = OColElem()
    o:add_to_world()
    o:set_color(color2)
    od:fill(o, i*0.035)
  end

  -- Random
  od = ODispenser()
  od:add_to_world()
  od:set_pos(table_size_x/2-wall_width/2, conf_disp==1 and disp_offset_y or -disp_offset_y, disp_offset_z, 1)
  for i = 1,5 do
    o = OColElem()
    o:add_to_world()
    o:set_color(color1)
    od:fill(o, i*0.035)
  end
  od = ODispenser()
  od:add_to_world()
  od:set_pos(-table_size_x/2+wall_width/2, conf_disp==1 and disp_offset_y or -disp_offset_y, disp_offset_z, 3)
  for i = 1,5 do
    o = OColElem()
    o:add_to_world()
    o:set_color(color2)
    od:fill(o, i*0.035)
  end


  -- Lintels and lintel storages
  trace("  lintels and lintel storages")

  ols = OLintelStorage()
  ols:add_to_world()
  ols:set_pos(-0.200, 0)
  ol = OLintel()
  ol:add_to_world()
  ol:set_color(color1)
  ols:fill(ol)

  ols = OLintelStorage()
  ols:add_to_world()
  ols:set_pos(-0.600, 0)
  ol = OLintel()
  ol:add_to_world()
  ol:set_color(color1)
  ols:fill(ol)

  ols = OLintelStorage()
  ols:add_to_world()
  ols:set_pos(0.200, 0)
  ol = OLintel()
  ol:add_to_world()
  ol:set_color(color2)
  ols:fill(ol)

  ols = OLintelStorage()
  ols:add_to_world()
  ols:set_pos(0.600, 0)
  ol = OLintel()
  ol:add_to_world()
  ol:set_color(color2)
  ols:fill(ol)
end

