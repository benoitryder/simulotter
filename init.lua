
-- Config

config.gravity_z = -0.5
config.cfm = config.cfm -- CFM depends on ODE precision
config.step_dt = 0.002
config.time_scale =  2
config.contacts_nb = 5
config.drop_epsilon = 0.001

config.draw_epsilon = 0.0005
config.draw_div = 20
config.draw_direction_r = 0.05
config.draw_direction_h = 0.10

config.perspective_fov = 45.0
config.perspective_near = 0.1
config.perspective_far = 300.0

config.screen_x = 800
config.screen_y = 600
config.fullscreen = false
config.fps = 60

config.bg_color = { 0.8, 0.8, 0.8 }
config.log_flush = true


--
-- Debug trace functions
--

function tracet(o)
  for k,v in pairs(o) do
    trace("  "..tostring(k)..": "..type(v))
  end
end

function traceo(o)
  t = type(o)
  trace("trace o ("..t.."):")
  if t == 'table' then
    tracet(o)
  elseif t == 'userdata' then
    tracemt(o)
  end
end

function tracemt(o)
  trace("trace mt:")
  tracet(getmetatable(o))
end


trace("------ SCRIPT START ------")

r1 = RBasic(.4, .4, .2, 10)
r1:init()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2)

x,y,z = r1:get_pos()
trace("R1: "..x..","..y..","..z)

r1:set_dv_max(100);
r1:set_dav_max(10);
r1:set_v_max(1.5)
r1:set_av_max(4)
r1:set_threshold_xy(0.05)
r1:set_threshold_a(0.10)


-- Building
if false then
  gcol = Geom:cylinder(0.035, 0.030)
  col = {}
  for i = 1,4 do
    col[i] = ObjectColor()
    col[i]:add_geom(gcol)
    col[i]:set_mass(0.100)
    col[i]:init()
  end

  glin = Geom:box(0.200, 0.070, 0.030)
  lin = ObjectColor()
  lin:add_geom(glin)
  lin:set_mass(0.300)
  lin:init()

  col[1]:set_color( { 1,0,0 } )
  col[2]:set_color( { 0,1,0 } )
  col[3]:set_color( { 1,0,0 } )
  col[4]:set_color( { 0,1,0 } )
  lin:set_color( { 1,1,0 } )

  col[1]:set_pos( -0.050, 0, 0.070 )
  col[2]:set_pos( -0.050, 0, 0.105 )
  col[3]:set_pos(  0.050, 0, 0.070 )
  col[4]:set_pos(  0.050, 0, 0.105 )
  lin:set_pos( 0, 0, 0.140 )
end


function r1:strategy()

  t = {
    {  .2,   -1, true },
    {  0,  -.5, false },
    {  1.2, 0, false },
    { self:get_x(), self:get_y(), false },
  }

  while true do
    for k,v in pairs(t) do
      self:order_xy( v[1], v[2], v[3] )
      repeat coroutine.yield() until self:is_waiting()
    end
  end
  self:order_stop()
  return
end


dofile("2009.lua")

r1:match_register()

trace("------ SCRIPT END ------")

