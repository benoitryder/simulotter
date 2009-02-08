
-- Config

config.gravity_z = -9.81
config.step_dt = 0.002
config.time_scale =  1
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

--r1 = RBasic(.2, .2, .1, 10)
r1 = RORobot(10)
r1:add_to_world()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2, 0.1001)

x,y,z = r1:get_pos()
trace("R1: "..x..","..y..","..z)

r1:set_v_max(1.0)
r1:set_av_max(4)
r1:set_threshold_xy(0.001)
r1:set_threshold_a(0.005)
r1:set_pachev_v(0.05)
r1:set_threshold_pachev(0.001)

function r1:strategy()

  self:order_pachev_move( 0.050 )

  self:order_xya( -1.1, 0.075, math.pi, false )
  repeat coroutine.yield() until self:is_waiting()
  self:set_v_max(0.2)
  self:order_back( 0.100 )
  repeat coroutine.yield() until self:is_waiting()
  self:order_pachev_release()
  self:order_pachev_move( 0 )

  self:order_back( 0.100 )
  repeat coroutine.yield() until self:is_waiting()
  self:order_pachev_grab()
  self:order_pachev_move( 1 )

  self:order_xy( -0.300, 0, true )
  self:set_v_max(0.8)
  repeat coroutine.yield() until self:is_waiting()
  self:order_xy( 0, 0.4, true )
  repeat coroutine.yield() until self:is_waiting()

  self:order_pachev_release()
  trace("END: stop robot")
  self:order_stop()
  return
end

function r1:strategy_()

  x,y = self:get_xy()
  t = {
    {  .2,   -1, true },
    {  0,  -.5, false },
    {  1.2, 0, false },
    {  x, y , false },
  }

  while true do
    for k,v in pairs(t) do
      self:order_xy( v[1], v[2], v[3] )
      repeat
        --trace("R1: "..tostring(x)..","..tostring(y))
        coroutine.yield()
      until self:is_waiting()
    end
  end
  trace("END: stop robot")
  self:order_stop()
  return
end

dofile("2009.lua")

r1:match_register()

trace("------ SCRIPT END ------")

