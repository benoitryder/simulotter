
-- Constants
local pix_size = 320

local color0 = { 0, 0, 0.8 }
local color1 = { 0, 0.8, 0 }
local color2 = { 0.8, 0, 0 }
local table_size_x = 3.0
local table_size_y = 2.1
local wall_width  = 0.070
local wall_height = 0.050
local start_c = 0.9
local robot_c = 0.20
local dot_r = 0.1
local cross_r = dot_r*1.2
-- Robot position
local rx, ry, rz, ra = 0.9, -0.6, robot_c, 0.92*math.pi

-- Config
config.bg_color = { 0.8, 0.8, 0.8 }

config.draw_epsilon = 0.0005
config.draw_div = 20

config.perspective_fov = 45.0
config.perspective_near = 0.1
config.perspective_far = 300.0

config.screen_x = pix_size
config.screen_y = pix_size
config.fullscreen = false
config.fps = 60
config.antialias = 6

physics = Physics()
display = Display()


require('modules/colors')

-- Camera
display:set_camera_mode(display.CAM.FIXED)
display:set_camera_target({ x=0.1, y=-0.2, z=0 })
display:set_camera_eye({ r=5, theta=0.6, phi=-1.0 })


---- Field

-- Return position on robot's axis
function raxis_pos(r)
  return rx+r*math.cos(ra), ry+r*math.sin(ra), rz
end


-- Robot
o = OSimple()
o:set_shape( Shape:box(robot_c,robot_c,robot_c) )
o:add_to_world()
o:set_color({1,1,0})
o:set_pos( rx, ry, rz+0.01 )
o:set_rot( ra, 0, 0 )

-- Dots
sh = Shape:sphere(dot_r)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_color(colors.white)
o:set_pos( raxis_pos(0.50) )
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_color(colors.white)
o:set_pos( raxis_pos(0.90) )

-- Cross
--sh = Shape:capsuleY(dot_r,3*dot_r)
sh = Shape:box(cross_r,3*cross_r,cross_r)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_color(colors.white)
o:set_pos( raxis_pos(1.50) )
o:set_rot( ra+math.pi/4, 0, 0 )
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_color(colors.white)
o:set_pos( raxis_pos(1.50) )
o:set_rot( ra+3*math.pi/4, 0, 0 )


-- Ground
trace("  ground")

o = OGround(color0, color1, color2)
o:add_to_world()

sh = Shape:box(start_c/2,start_c/2,config.draw_epsilon*2)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos( -(table_size_x-start_c)/2, (table_size_y-start_c)/2, 0 )
o:set_color(color1)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos( (table_size_x-start_c)/2, (table_size_y-start_c)/2, 0 )
o:set_color(color2)


-- Walls (N, S, E, W)
trace("  walls")

sh = Shape:box(table_size_x/2+wall_width, wall_width/2, wall_height/2)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos(0, table_size_y/2+wall_width/2, wall_height/2)
o:set_color(colors.black)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos(0, -table_size_y/2-wall_width/2, wall_height/2)
o:set_color(colors.black)

sh = Shape:box(wall_width/2, table_size_y/2+wall_width, wall_height/2)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos(table_size_x/2+wall_width/2, 0, wall_height/2)
o:set_color(colors.black)
o = OSimple()
o:set_shape( sh )
o:add_to_world()
o:set_pos(-table_size_x/2-wall_width/2, 0, wall_height/2)
o:set_color(colors.black)

