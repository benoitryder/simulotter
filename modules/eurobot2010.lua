
-- Eurobot 2010: Feed the World

require('modules/colors')
require('modules/eurobot')
module('eurobot2010', package.seeall)

-- Team colors
color1 = colors.ral_5005
color2 = colors.ral_1023

-- Various constants
TABLE = eurobot.TABLE
WALL  = eurobot.WALL

FIELD = { dx = 0.450, dy = 0.250, oy = 0.128 }
RAISED = { z = 0.140 }
BAC = {
  ox = (TABLE.sx-0.500)/2, oy = -TABLE.sy/2,
  sx = 0.500, sy = 0.300, h = 0.300, w = 0.010
}

-- Game objects (filled in init)
tomatoes = {}
corns = {}
oranges = {}


-- Compute field element position, (0,0) is middle bottom
function field_pos(x,y)
  return x*FIELD.dx, -TABLE.sy/2+FIELD.oy+y*FIELD.dy
end

-- Return team number if an object is in a bac, false otherwise
-- Test is based on object's center position.
function is_collected(o)
  local x,y,z = o:get_pos()
  if z < 0 and z > -BAC.h and y > BAC.oy-BAC.sy and y < BAC.oy then
    if x > TABLE.sx/2-BAC.sx and x < TABLE.sx/2 then
      return 1
    elseif x < -TABLE.sx/2+BAC.sx and x > -TABLE.sx/2 then
      return 2
    end
  end
  return false
end

-- Compute score of each team, in grams, returned as a table
function compute_scores()

  local sc = { 0, 0 } -- team scores
  for k,v in ipairs(tomatoes) do
    local t = is_collected(v)
    if t then sc[t] = sc[t] + 150 end
  end
  for k,v in ipairs(corns) do
    local t = is_collected(v)
    if t then sc[t] = sc[t] + 250 end
  end
  for k,v in ipairs(oranges) do
    local t = is_collected(v)
    if t then sc[t] = sc[t] + 300 end
  end

  return sc

end


-- Add a tomato at given position
function add_tomato(x,y)
  local o = OTomato()
  o:add_to_world()
  o:set_pos( field_pos(x,y) )
  tomatoes[#tomatoes+1] = o
end

-- Add a corn at given position, fake if f is true
function add_corn(x,y,f)
  local o = f and OCornFake() or OCorn()
  o:add_to_world()
  local ox, oy = field_pos(x,y)
  if f then
    o:set_pos( ox, oy )
  else
    o:plant( ox, oy )
  end
  corns[#corns+1] = o
end


-- Branches and trees classes
do

  -- Branch class
  OBranch = class(OSimple, function(self,h)
    OSimple._ctor(self)
    self:set_shape( self.shape )
    -- actual color differs from rules
    -- self:set_color(colors.white)
    self:set_color(colors.plexi)
    self.h = h
  end)
  -- Make all trees with the same height but use different z position
  OBranch.sh_h = 0.25
  OBranch.shape = Shape:cylinderZ(0.025, OBranch.sh_h/2)

  function OBranch.add_orange(self)
    local x,y,z = self:get_pos()
    local o = OOrange()
    o:add_to_world()
    o:set_mass(0)  --XXX oranges are static, for now
    o:set_pos(x, y, RAISED.z+self.h+0.050*math.cos(math.asin(0.025/0.050)))
    oranges[#oranges+1] = o
  end

  -- Compute tree position (x, y and z)
  -- x and y are -1 or 1 and give tree position
  function branch_pos(x,y,h)
    assert( x==1 or x==-1 and y==1 or y==-1, "invalid tree position")
    assert( h==0.25 or h==0.20 or h==0.15, "invalid tree height")
    local dx = ({[0.25]=0, [0.20]=0.055, [0.15]=0.080})[h]
    local dy = ({[0.25]=0, [0.20]=0.075, [0.15]=-0.050})[h]
    local px = x * (0.500/2-0.080-0.055 + dx)
    local py = (y==1 and 0.250-0.070-0.075 or -0.250+0.080+0.050) + dy + TABLE.sy/2-0.250
    local pz = RAISED.z-OBranch.sh_h/2+h
    return px, py, pz
  end

  -- Add a tree (3 branches and their oranges)
  -- x and y are -1 or 1 and give tree position
  function add_tree(x, y)
    local o
    for k,h in ipairs({0.25, 0.20, 0.15}) do
      o = OBranch(h)
      o:add_to_world()
      o:set_pos( branch_pos(x, y, h) )
      o:add_orange()
    end
  end

end


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
    conf_side   = fconf % 16
    conf_center = math.floor(fconf/16)
  end
  if conf_side < 1 or conf_side > 9 or conf_center < 1 or conf_center > 4 then
    error("invalid field configuration")
  end
  trace("Feed the World rules, fake corns: lateral: "..tostring(conf_side)..", central:"..tostring(conf_center))


  -- Add a collect bac, side is -1 (x<0) or 1 (x>0)
  local bac_sh = nil  -- reuse shape
  function add_bac(side)
    local c = (side == 1 and color1 or color2)
    local off_x = side * BAC.ox
    local o

    -- First call: create shapes
    if bac_sh == nil then
      local b_side = Shape:box(BAC.w/2, BAC.sy/2, WALL.h/2)
      local b_back = Shape:box(BAC.sx/2+BAC.w, BAC.w/2, WALL.h/2)
      local p_side = Shape:box(BAC.w/2, BAC.sy/2, BAC.h/2)
      local p_back = Shape:box(BAC.sx/2+BAC.w, BAC.w/2, BAC.h/2)
      local p_front = Shape:box(BAC.sx/2+BAC.w, BAC.w/2, BAC.h/2-0.020)
      local p_bottom = Shape:box(BAC.sx/2+BAC.w, (BAC.sy+BAC.w)/2, WALL.h/2)
      bac_sh = {
        band = Shape:compound({
          {b_side, {-BAC.sx/2-BAC.w/2, -BAC.sy/2, 0}},
          {b_side, { BAC.sx/2+BAC.w/2, -BAC.sy/2, 0}},
          {b_back, { 0, -BAC.sy-BAC.w/2, 0}},
        }),
        plexi = Shape:compound({
          {p_side,   {-BAC.sx/2-BAC.w/2, -BAC.sy/2, 0}},
          {p_side,   { BAC.sx/2+BAC.w/2, -BAC.sy/2, 0}},
          {p_back,   {0, -BAC.sy-BAC.w/2, 0}},
          {p_front,  {0, -BAC.w/2, -0.020}},
          {p_bottom, {0, -BAC.sy/2-BAC.w/2, -BAC.h/2}},
        }),
      }
    end

    o = OSimple()
    o:set_shape( bac_sh.band )
    o:add_to_world()
    o:set_pos(off_x, BAC.oy, WALL.h/2)
    o:set_color(c)

    o = OSimple()
    o:set_shape( bac_sh.plexi )
    o:add_to_world()
    o:set_pos(off_x, BAC.oy, -BAC.h/2)
    o:set_color(colors.plexi)
  end


  local o, ground, sh


  -- Ground and raised zone
  trace("  ground")

  ground = OGround(colors.ral_6018, color1, color2)
  ground:add_to_world()
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
  o:set_shape(Shape:box(TABLE.sx/2-BAC.sx, WALL.w/2, WALL.h/2))
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
    add_tree(-1,-1)
    add_tree(-1, 1)
    add_tree( 1,-1)
    add_tree( 1, 1)
  end

end

