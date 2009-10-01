
-- Eurobot 2010: Feed the World

require('modules/colors')
require('modules/eurobot')
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


  -- Various constants

  TABLE = eurobot.TABLE
  WALL  = eurobot.WALL

  FIELD = { dx = 0.450, dy = 0.250, oy = 0.128 }
  RAISED = { z = 0.140 }


  -- Compute field element position, (0,0) is middle bottom
  function field_pos(x,y)
    return x*FIELD.dx, -TABLE.sy/2+FIELD.oy+y*FIELD.dy
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
    o:set_mass(0) --TODO temporary work around
    o:add_to_world()
    o:set_color( f and colors.ral_9017 or colors.ral_1013 )
    o:set_pos( field_pos(x,y) )
  end

  -- Add a collect bac, side is -1 (x<0) or 1 (x>0)
  local bac_sh = nil  -- reuse shape
  function add_bac(side)
    local c = (side == 1 and color1 or color2)
    local off_x = side * (TABLE.sx-0.500)/2
    local off_y = -TABLE.sy/2
    local bac_l = 0.300
    local bac_h = 0.300
    local bac_w = 0.010
    local o

    -- First call: create shapes
    if bac_sh == nil then
      bac_sh = {
        b_side = Shape:box(bac_w/2, bac_l/2, WALL.h/2),
        b_back = Shape:box(0.250+bac_w, bac_w/2, WALL.h/2),
        p_side = Shape:box(bac_w/2, bac_l/2, bac_h/2),
        p_back = Shape:box(0.250+bac_w, bac_w/2, bac_h/2),
        p_front = Shape:box(0.250+bac_w, bac_w/2, bac_h/2-0.020),
        p_bottom = Shape:box(0.250+bac_w, (bac_l+bac_w)/2, WALL.h/2),
      }
    end

    -- color bands (W, E, S)
    o = OSimple()
    o:set_shape( bac_sh.b_side )
    o:add_to_world()
    o:set_pos(off_x-0.250-bac_w/2, off_y-bac_l/2, WALL.h/2)
    o:set_color(c)
    o = OSimple()
    o:set_shape( bac_sh.b_side )
    o:add_to_world( bac_sh.b_side )
    o:set_pos(off_x+0.250+bac_w/2, off_y-bac_l/2, WALL.h/2)
    o:set_color(c)
    o = OSimple()
    o:set_shape( bac_sh.b_back )
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l-bac_w/2, WALL.h/2)
    o:set_color(c)

    -- plexi (W, E, S, N, bottom)
    o = OSimple()
    o:set_shape( bac_sh.p_side )
    o:add_to_world()
    o:set_pos(off_x-0.250-bac_w/2, off_y-bac_l/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape( bac_sh.p_side )
    o:add_to_world()
    o:set_pos(off_x+0.250+bac_w/2, off_y-bac_l/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape( bac_sh.p_back )
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l-bac_w/2, -bac_h/2)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape( bac_sh.p_front )
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_w/2, -bac_h/2-0.020)
    o:set_color(colors.plexi)
    o = OSimple()
    o:set_shape( bac_sh.p_bottom )
    o:add_to_world()
    o:set_pos(off_x, off_y-bac_l/2-bac_w/2, -bac_h)
    o:set_color(colors.plexi)
  end


  local o, sh


  -- Ground and raised zone
  trace("  ground")

  o = OGround(colors.ral_6018, color1, color2)
  o:add_to_world()
  o = ORaisedZone()
  o:set_pos(0, TABLE.sy/2-0.250, 0)
  o:add_to_world()


  -- Walls (N, E, W, S)
  trace("  walls")

  o = OSimple()
  o:set_shape(Shape:box(TABLE.sx/2+WALL.w, WALL.w/2, WALL.h/2))
  o:add_to_world()
  o:set_pos(0, TABLE.sy/2+WALL.w/2, WALL.h/2)
  o:set_color(colors.ral_9017)
  sh = Shape:box(WALL.w/2, TABLE.sy/2+WALL.w, WALL.h/2)
  o = OSimple()
  o:set_shape( sh )
  o:add_to_world()
  o:set_pos(TABLE.sx/2+WALL.w/2, 0, WALL.h/2)
  o:set_color(colors.ral_9017)
  o = OSimple()
  o:set_shape( sh )
  o:add_to_world()
  o:set_pos(-TABLE.sx/2-WALL.w/2, 0, WALL.h/2)
  o:set_color(colors.ral_9017)
  o = OSimple()
  o:set_shape(Shape:box(2.0/2, WALL.w/2, WALL.h/2))
  o:add_to_world()
  o:set_pos(0, -TABLE.sy/2-WALL.w/2, WALL.h/2)
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

  -- Oranges and trees
  trace("  oranges and trees")
  do

    -- Branch class
    OBranch = class(OSimple, function(self,h)
      OSimple._ctor(self)
      self:set_shape( self.shape )
      self:add_to_world()
      self:set_color(colors.white)
      self.h = h
    end)
    -- Make all trees with the same height but use different z position
    OBranch.sh_h = 0.25
    OBranch.shape = Shape:cylinderZ(0.025, OBranch.sh_h/2)
    function OBranch.set_top_pos(self, x, y)
      self:set_pos(x, y+TABLE.sy/2-0.250, RAISED.z-OBranch.sh_h/2+self.h)
    end
    function OBranch.add_orange(self)
      local x,y,z = self:get_pos()
      local o = OOrange()
      o:add_to_world()
      o:set_mass(0)  --XXX oranges are static, for now
      o:set_pos(x, y, RAISED.z+self.h+0.050*math.cos(math.asin(0.025/0.050)))
    end

    -- Tree class
    OTree = class(OSimple, function(self, x, y)
      local o
      for k,v in ipairs(self.branches) do
        o = OBranch(v.h)
        o:set_top_pos( (x<0 and x-v.x or x+v.x), y+v.y )
        o:add_orange()
      end
    end)
    -- branch offset (origin at the tallest branch)
    OTree.branches = {
      { h=0.25, x=0, y=0 },
      { h=0.20, x=0.055, y=0.075 },
      { h=0.15, x=0.080, y=-0.050 },
    }

    local tree_offset_x = 0.500/2-0.080-0.055
    local tree_offset_y = 0.100

    OTree( tree_offset_x,  0.250-0.070-0.075)
    OTree(-tree_offset_x,  0.250-0.070-0.075)
    OTree( tree_offset_x, -0.250+0.080+0.050)
    OTree(-tree_offset_x, -0.250+0.080+0.050)

  end

end

