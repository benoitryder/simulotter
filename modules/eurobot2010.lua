
-- Eurobot 2010: Feed the World

require('modules/colors')
module('eurobot2010', package.seeall)

-- Team colors
color1 = colors.ral_5005
color2 = colors.ral_1023

-- Match init
function init(fconf)
  -- Field configuration:
  --   lateral: 1 to 9
  --   central: (1 to 4) * 16
  -- Can be defined as 2-digit hexadecimal value: 0x<side><center>
  local conf_side, conf_center
  if fconf == nil or fconf < 0 then
    conf_side   = math.random(9)
    conf_center = math.random(4)
  else
    conf_side   = fconf % 16 + 1
    conf_center = math.floor(fconf/16) + 1
  end
  if conf_side < 1 or conf_side > 9 or conf_center < 1 or conf_center > 4 then
    error("invalid field configuration")
  end
  trace("Feed the World rules, fake corns: lateral: "..tostring(conf_side)..", central:"..tostring(conf_center))


  -- Various variables

  local table_size_x = 3.0
  local table_size_y = 2.1

  local field_space_x = 0.450
  local field_space_y = 0.250
  local field_offset_y = 0.128

  local wall_width  = 0.022
  local wall_height = 0.070

  -- Compute field element position, (0,0) is middle bottom
  function field_pos(x,y)
    return x*field_space_x, -table_size_y/2+field_offset_y+y*field_space_y
  end

  -- Add a tomato at given position
  function add_tomato(x,y)
    local o = OTomato()
    o:add_to_world()
    o:set_pos( field_pos(x,y) )
  end
  -- Add a corn at given position, fake if f is true
  function add_corn(x,y,f)
    local o = OCorn()
    if f then o:set_mass(0) end --TODO temporary work around
    o:add_to_world()
    o:set_color( f and colors.ral_9017 or colors.ral_1013 )
    o:set_pos( field_pos(x,y) )
  end

  -- Add a collect bac, side is -1 (x<0) or 1 (x>0)
  function add_bac(side)
    local c = (side == 1 and color1 or color2)
    local off_x = side * (table_size_x-0.500)/2
    local off_y = -table_size_y/2
    local bac_l = 0.300
    local bac_h = 0.300
    local bac_w = 0.010
    local o

    -- color bands (W, E, S)
    o = OSimple()
    o:set_shape(Shape:box(bac_w/2, bac_l/2, wall_height/2))
    o:add_to_world()
    o:set_pos(off_x-0.250-bac_w/2, off_y-bac_l/2, wall_height/2)
    o:set_color(c)
    o = OSimple()
    o:set_shape(Shape:box(bac_w/2, bac_l/2, wall_height/2))
    o:add_to_world()
    o:set_pos(off_x+0.250+bac_w/2, off_y-bac_l/2, wall_height/2)
    o:set_color(c)
    o = OSimple()
    o:set_shape(Shape:box(0.250+bac_w, bac_w/2, wall_height/2))
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l-bac_w/2, wall_height/2)
    o:set_color(c)

    -- plexi (W, E, S, N, bottom)
    o = OSimple()
    o:set_shape(Shape:box(bac_w/2, bac_l/2, bac_h/2))
    o:add_to_world()
    o:set_pos(off_x-0.250-bac_w/2, off_y-bac_l/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape(Shape:box(bac_w/2, bac_l/2, bac_h/2))
    o:add_to_world()
    o:set_pos(off_x+0.250+bac_w/2, off_y-bac_l/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape(Shape:box(0.250+bac_w, bac_w/2, bac_h/2))
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l-bac_w/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape(Shape:box(0.250+bac_w, bac_w/2, bac_h/2-0.020))
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_w/2, -bac_h/2-0.020)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape(Shape:box(0.250+bac_w, (bac_l+bac_w)/2, wall_height/2))
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l/2-bac_w/2, -bac_h)
    o:set_color(colors.plexi)
  end


  local o


  -- Ground and raised zone
  trace("  ground")

  o = OGround(colors.ral_6018, color1, color2)
  o:add_to_world()
  o = ORaisedZone()
  o:set_pos(0, table_size_y/2-0.250, 0)
  o:add_to_world()


  -- Walls (N, E, W, S)
  trace("  walls")

  o = OSimple()
  o:set_shape(Shape:box(table_size_x/2+wall_width, wall_width/2, wall_height/2))
  o:add_to_world()
  o:set_pos(0, table_size_y/2+wall_width/2, wall_height/2)
  o:set_color(colors.ral_9017)
  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, table_size_y/2+wall_width, wall_height/2))
  o:add_to_world()
  o:set_pos(table_size_x/2+wall_width/2, 0, wall_height/2)
  o:set_color(colors.ral_9017)
  o = OSimple()
  o:set_shape(Shape:box(wall_width/2, table_size_y/2+wall_width, wall_height/2))
  o:add_to_world()
  o:set_pos(-table_size_x/2-wall_width/2, 0, wall_height/2)
  o:set_color(colors.ral_9017)
  o = OSimple()
  o:set_shape(Shape:box(2.0/2, wall_width/2, wall_height/2))
  o:add_to_world()
  o:set_pos(0, -table_size_y/2-wall_width/2, wall_height/2)
  o:set_color(colors.ral_9017)

  add_bac(-1)
  add_bac( 1)


  -- Corns
  trace("  corns")
  do
    local fake_pos_side = {
      -- {x1,y1} , {x2,y2}
      { {2,2},{3,3} },
      { {2,2},{3,5} },
      { {2,2},{3,1} },
      { {2,4},{3,1} },
      { {2,4},{3,5} },
      { {2,4},{3,3} },
      { {1,3},{3,3} },
      { {1,3},{3,5} },
      { {1,3},{3,1} },
    }
    local fake_pos_center = {
      -- y1 , {x2,y2}  (x1 = 0)
      { 2,{2,0} },
      { 2,{1,1} },
      { 0,{2,0} },
      { 0,{1,1} },
    }

    -- fakes
    local t1 = fake_pos_center[conf_center]
    local t2 = fake_pos_side[conf_side]
    local fakes = { [0]= t1[1], [1]= t1[2], [2]= t2[1], [3]= t2[2] }

    add_corn(0, 0, fakes[0]==0)
    add_corn(0, 2, fakes[0]==2)
    for i=1,3 do
      for j=i+2,0,-2 do
        local f = false
        for k,v in ipairs(fakes) do
          if v[1]==i and v[2]==j then
            f = true
            break
          end
        end
        add_corn( i,j,f)
        add_corn(-i,j,f)
      end
    end
  end

  -- Tomatoes
  trace("  tomatoes")
  do
    add_tomato(0,1)
    add_tomato(0,3)
    for i=1,3 do
      add_tomato( i,i-1)
      add_tomato( i,i+1)
      add_tomato(-i,i-1)
      add_tomato(-i,i+1)
    end
  end

end

