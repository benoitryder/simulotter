
-- Config

config.gravity_z = -0.5
config.step_dt = 0.001
config.contacts_nb = 5
config.drop_epsilon = 0.001

config.draw_epsilon = 0.0001
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

r1 = RBasic(.4, .4, .2, 3)
r1:set_pos(1, 0)


trace("------ SCRIPT END ------")

