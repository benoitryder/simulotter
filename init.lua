
-- Config

config.gravity_z = -9.81
config.step_dt = 0.002
config.time_scale = 0.5
config.match_fconf = -1
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

physics = Physics()
display = Display()


trace("------ SCRIPT START ------")

r1 = RORobot(10)
r1:add_to_world()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2, 0.1001)

x,y,z = r1:get_pos()
trace("R1: "..x..","..y..","..z)

r1:set_v_max(0.5)
r1:set_av_max(4)
r1:set_threshold_xy(0.001)
r1:set_threshold_a(0.005)
r1:set_pachev_v(0.1)
r1:set_pachev_eject_v(0.001)
r1:set_threshold_pachev(0.001)

config.match_fconf = 0x03
function r1:strategy()

  -- Collect
  for i = 0, 3 do
    trace("collect elem "..tostring(i))
    self:order_pachev_move( 0.060 )
    self:order_xya( -1.2, 0.475-i*0.200, math.pi, false )
    repeat coroutine.yield() until self:is_waiting()
    self:set_v_max(0.2)
    self:order_back( 0.200 )
    repeat coroutine.yield() until self:is_waiting()
    for j = 0, 300 do coroutine.yield() end
    self:order_pachev_release()
    repeat coroutine.yield() until self:is_waiting()
    self:set_v_max(0.5)
    self:order_pachev_move( 0 )
    repeat coroutine.yield() until self:is_waiting()
    self:order_pachev_grab()
    self:order_xy( -0.200, 0, true )
    repeat coroutine.yield() until self:is_waiting()
  end
  trace("all collected")

  self:order_xya( -0.4, 0, math.pi, false )
  self:order_pachev_move( 0.100 )
  repeat coroutine.yield() until self:is_waiting()
  self:order_back( 0.200 )
  repeat coroutine.yield() until self:is_waiting()
  for i = 0, 500 do coroutine.yield() end
  self:order_pachev_release()
  for i = 0, 20 do coroutine.yield() end
  self:order_pachev_eject()
  repeat coroutine.yield() until self:is_waiting()

  for i = 0, 500 do coroutine.yield() end
  trace("go away")
  self:order_xy( -1.0, 0, true )
  repeat coroutine.yield() until self:is_waiting()

  trace("END: stop robot")
  self:order_stop()
  return
end

dofile("2009.lua")

r1:match_register()

trace("------ SCRIPT END ------")

