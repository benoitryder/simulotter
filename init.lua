
-- Config

config.gravity_z = -9.81
config.step_dt = 0.003
config.time_scale = 1
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
config.antialias = 0

config.bg_color = { 0.8, 0.8, 0.8 }
config.log_flush = true

physics = Physics()
display = Display()
display:init()


trace("------ SCRIPT START ------")

require('modules/eurobot2010')

r1 = eurobot2010.Galipeur(3.2)
r1:add_to_world()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2, 0.1001)
r1:set_rot( -math.pi/2, 0, 0 )

r1:set_speed_xy(0.4, 1.33);
r1:set_speed_a(2.46, 12.3)
r1:set_threshold_xy(0.003)
r1:set_threshold_a(0.01*math.pi)

osd = OSD()
osd.x, osd.y = 10, 20
osd.color = {0, 0, 0}
function osd.text()
  x,y,z = r1:get_pos()
  a,_,_ = r1:get_rot()
  return string.format("%3.3fs | R1: %+1.3f , %+1.3f  %+1.3f", physics:get_time(), x,y,a)
end
osd:show()

-- Scores, refreshed every (simulated) second
do 
  local scores = { 0, 0 }
  local osd = OSD()
  osd.x, osd.y = 400,20
  osd.color = {0, 0, 0}
  function osd.text()
    return string.format("scores: %4d - %4d", scores[1], scores[2])
  end
  osd:show()
  local task = Task(1)
  function task.callback()
    scores = eurobot2010.compute_scores()
  end
  task:schedule()
end

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


function r1:strategy()

  self:order_xy( eurobot2010.field_pos(-2,5) )
  repeat coroutine.yield() until self:is_waiting()
  local posx, posy = eurobot2010.field_pos(-1,4)
  self:order_xya( posx, posy, math.atan(eurobot2010.FIELD.dy/eurobot2010.FIELD.dx) )
  repeat coroutine.yield() until self:is_waiting()
  self:order_xy( eurobot2010.field_pos(2,1) )
  repeat coroutine.yield() until self:is_waiting()

  trace("END: stop robot")
  self:order_stop()
  return

end

-- Schedule r1 tasks: asserv then strategy
local task
task = Task(config.step_dt)
task.callback = function() r1:asserv() end
task:schedule()
task = Task(config.step_dt)
task.callback = coroutine.create(function() r1:strategy() end)
task:schedule()


eurobot2010.init( nil )

-- Rotate through camera with with 'c'
do
  local cam_modes = {
    -- mode, eye, target
    { Display.CAM.FIXED, { r=4, theta=math.pi/6, phi=-math.pi/3 }, {x=0,y=0,z=0} },
    { Display.CAM.FREE, nil, nil },
    { Display.CAM.FOLLOW, nil, { obj=r1 } },
    { Display.CAM.ONBOARD, { obj=r1, x=0,y=0,z=0.2}, { theta=0.6*math.pi, phi=0 } },
  }

  local cam_modes_iter = coroutine.create(function ()
    repeat for _,v in ipairs(cam_modes) do
      m,e,t = unpack(v)
      -- set objs before to avoid errors when switching mode
      if e and e.obj then display:set_camera_eye{obj=e.obj} end
      if t and t.obj then display:set_camera_target{obj=t.obj} end

      if m then display:set_camera_mode(m) end
      if e then display:set_camera_eye(e) end
      if t then display:set_camera_target(t) end
      coroutine.yield()
    end until nil
  end)

  -- set starting mode
  coroutine.resume(cam_modes_iter)

  display:set_handler({t=display.EVENT.KEYUP, key='c'}, function(ev)
    coroutine.resume(cam_modes_iter)
  end)
end

-- Screenshots with 'o'
local screen_n = 0
display:set_handler({ t=display.EVENT.KEYUP, key='o'} , function(ev)
  display:save_screenshot(string.format('screenshot%03d.png', screen_n))
  screen_n = screen_n + 1
end)


trace("------ SCRIPT END ------")

