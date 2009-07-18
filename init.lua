
-- Config

config.gravity_z = -9.81
config.step_dt = 0.003
config.time_scale = 1
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


r1 = Galipeur2009(10)
r1:add_to_world()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2, 0.1001)

x,y,z = r1:get_pos()
trace("R1: "..x..","..y..","..z)

r1:set_v_max(0.8)
r1:set_av_max(4)
r1:set_threshold_xy(0.001)
r1:set_threshold_a(0.005)
r1:set_pachev_v(0.2)
r1:set_pachev_eject_v(0.001)
r1:set_threshold_pachev(0.001)

osd = OSD()
osd.x, osd.y = 10, 20
osd.color = {0, 0, 0}
function osd.text()
  x,y,z = r1:get_pos()
  a,_,_ = r1:get_rot()
  return string.format("%3.3fs | R1: %+1.3f , %+1.3f  %+1.3f", physics:get_time(), x,y,a)
end
osd:show()

-- sharps
do
  local x, y, r
  x = math.cos(math.pi/3)
  y = math.sin(math.pi/3)
  r = 0.1
  r1:set_sharps({
    { r*x, r*y,0.10, 0,0.25, math.pi/3 },
    { r*x,-r*y,0.10, 0,0.25,-math.pi/3 }
  })

  osd_sharps = OSD()
  osd_sharps.x, osd_sharps.y = 10, 40
  osd_sharps.color = {0, 0, 0}
  osd_sharps.text = ""
  osd_sharps:show()

  local task = Task(0.1)
  local n = r1:get_sharp_count()
  function task.callback()
    txt = {}
    for i=1,n do
      local s = r1:test_sharp(i)
      if s then
        txt[#txt+1] = string.format('%1.3f', s)
      else
        txt[#txt+1] = '-----'
      end
    end
    osd_sharps.text = 'sharps: ' .. table.concat(txt, ' , ')
  end
  task:schedule()
end


config.match_fconf = 0x03
function r1:strategy()

  display:set_camera_target({ obj=r1 })
  display:set_camera_mode('LOOK')
  display:set_camera_eye({ x=-0.4, y=-0.5, z=1.5 })

  -- Collect
  for i = 0, 3 do
    trace("collect elem "..tostring(i))
    self:order_pachev_move( 0.060 )
    self:order_xya( -1.2, 0.475-i*0.200, math.pi, false )
    repeat coroutine.yield() until self:is_waiting()
    self:set_v_max(0.2)
    self:order_xy( 0.200, 0, true )
    repeat coroutine.yield() until self:is_waiting()
    for j = 0, 100 do coroutine.yield() end
    self:order_pachev_release()
    repeat coroutine.yield() until self:is_waiting()
    self:set_v_max(0.8)
    self:order_pachev_move( 0 )
    repeat coroutine.yield() until self:is_waiting()
    self:order_pachev_grab()
    self:order_xy( -0.200, 0, true )
    repeat coroutine.yield() until self:is_waiting()
  end
  trace("all collected")

  display:set_camera_mode('FIXED')
  display:set_camera_eye({ r=1.2, theta=1.2, phi=-0.5 })
  display:set_camera_target({ x=0, y=0, z=0.1 })


  self:order_xya( -0.4, 0, math.pi, false )
  self:order_pachev_move( 0.100 )
  repeat coroutine.yield() until self:is_waiting()
  self:order_xy( 0.150, 0, true )
  repeat coroutine.yield() until self:is_waiting()
  for i = 0, 200 do coroutine.yield() end
  self:order_pachev_release()
  for i = 0, 20 do coroutine.yield() end
  self:order_pachev_eject()
  repeat coroutine.yield() until self:is_waiting()

  for i = 0, 200 do coroutine.yield() end
  trace("go away")
  self:order_xy( -0.8, 0, true )
  repeat coroutine.yield() until self:is_waiting()

  trace("END: stop robot")
  self:order_stop()

  return
end

dofile("2009.lua")

r1:match_register()

r2 = Galipeur(10)
r2:add_to_world()
r2:set_pos((3.0-.5)/2, (2.1-.5)/2, 0.1001)
r2:set_v_max(0.5)
r2:set_av_max(4)
r2:set_threshold_xy(0.001)
r2:set_threshold_a(0.005)
r2:match_register()

function r2:update()
  return
end
function r2:strategy()
  return
end



trace("------ SCRIPT END ------")

