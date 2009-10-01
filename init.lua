
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


trace("------ SCRIPT START ------")

require('modules/eurobot2010')

r1 = eurobot2010.Galipeur(10)
r1:add_to_world()
r1:set_pos(-(3.0-.5)/2, (2.1-.5)/2, 0.1001)
r1:set_rot( -math.pi/2, 0, 0 )

x,y,z = r1:get_pos()
trace("R1: "..x..","..y..","..z)

r1:set_v_max(0.8)
r1:set_av_max(4)
r1:set_threshold_xy(0.001)
r1:set_threshold_a(0.010)

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


function r1:strategy()

  self:order_xy( eurobot2010.field_pos(-2,5) )
  repeat coroutine.yield() until self:is_waiting()
  self:order_xy( eurobot2010.field_pos(-1,4) )
  repeat coroutine.yield() until self:is_waiting()
  self:order_a( math.atan(eurobot2010.FIELD.dy/eurobot2010.FIELD.dx) )
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


trace("------ SCRIPT END ------")

