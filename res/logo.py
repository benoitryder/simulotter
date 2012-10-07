#!/usr/bin/env python

import sys, os
sys.path.append('.')  # allow to run from project root
import math
from simulotter import *


# Logo configuration constants

color0 = Color4(0,0,0.8)
color1 = Color4(0,0.8,0)
color2 = Color4(0.8,0,0)

table_size = vec2(3.0, 2.1)
wall_width  = 0.070
wall_height = 0.050
start_c = 0.9
robot_c = 0.20
dot_r = 0.1
cross_r = dot_r * 1.2

robot_tr = trans( quat(0, 0, 0.92*math.pi), vec3(0.9, -0.6, robot_c) )


def build_logo(ph):
  """Add logo objects to a Physics instance."""

  # Robot
  sh = ShBox(vec3(robot_c,robot_c,robot_c))
  robot = OSimple(sh)
  robot.color = Color4(1.,1.,0.)
  robot.trans = robot_tr
  robot.addToWorld(ph)

  # Dots
  sh = ShSphere(dot_r)
  for x in (0.50, 0.90):
    o = OSimple(sh)
    o.color = Color4.white()
    o.pos = robot_tr * vec3(x, 0, 0)
    o.addToWorld(ph)

  # Cross
  sh = ShBox(vec3(cross_r,3*cross_r,cross_r))
  for a in (math.pi/4, 3*math.pi/4):
    o = OSimple(sh)
    o.color = Color4.white()
    o.pos = robot_tr * vec3(1.50, 0, 0)
    o.rot = robot_tr.basis * quat(0, 0, a)
    o.addToWorld(ph)

  # Ground
  o = OGroundSquareStart(color0, color1, color2)
  o.addToWorld(ph)

  sh = ShBox(vec3(start_c*0.5, start_c*0.5, Display.draw_epsilon*2))
  v = 0.5 * ( table_size - vec2(start_c, start_c) )
  o = OSimple(sh)
  o.color = color1
  o.pos = vec2(-v.x, v.y)
  o.addToWorld(ph)
  o = OSimple(sh)
  o.color = color2
  o.pos = v
  o.addToWorld(ph)

  # Walls (N, S, E, W)
  sh = ShBox(0.5*vec3(table_size.x+2*wall_width, wall_width, wall_height))
  o = OSimple(sh)
  o.color = Color4.black()
  o.pos = 0.5*vec3(0, table_size.y+wall_width, wall_height)
  o.addToWorld(ph)
  o = OSimple(sh)
  o.color = Color4.black()
  o.pos = 0.5*vec3(0, -table_size.y-wall_width, wall_height)
  o.addToWorld(ph)

  sh = ShBox(0.5*vec3(wall_width, table_size.y+2*wall_width, wall_height))
  o = OSimple(sh)
  o.color = Color4.black()
  o.pos = 0.5*vec3(table_size.x+wall_width, 0, wall_height)
  o.addToWorld(ph)
  o = OSimple(sh)
  o.color = Color4.black()
  o.pos = 0.5*vec3(-table_size.x-wall_width, 0, wall_height)
  o.addToWorld(ph)


def set_camera(di):
  """Set camera to look on the logo."""
  di.cam_mode = di.FIXED
  di.cam_target.cart = vec3(0.1,-0.2,0)
  di.cam_eye.spheric = spheric3(5,0.6,-1.0)


if __name__ == '__main__':
  from optparse import OptionParser

  parser = OptionParser(
      description="SimulOtter logo builder.",
      usage="%prog [OPTIONS] [FILE]")
  parser.add_option('-s', '--size', dest='size', metavar='X[,Y]',
      help="logo size (default: 320,320")
  parser.add_option('-a', '--antialias', dest='antialias', metavar='N',
      help="antialiasing level (default: 0, disabled)")
  parser.add_option('-b', '--background', dest='background', metavar='RGB',
      help="background color, as an RRGGBB hexa string (default: cccccc)")
  parser.set_defaults(
      size='320',
      antialias=0,
      background='cccccc',
      )
  opts, args = parser.parse_args()

  if ',' in opts.size:
    size = [ int(x) for x in opts.size.split(',',2) ]
  else:
    size = [ int(opts.size) ] * 2
  antialias = int(opts.antialias)
  if len(opts.background) != 6:
    parser.error("invalid background color format")
  bg_color = Color4( *(int(opts.background[i:i+2],16) for i in (0,1,2) ) )

  if len(args) == 0:
    fout = None
  elif len(args) == 1:
    fout = args[0]
  else:
    parser.error("extra arguments")

  Display.antialias = antialias
  ph = Physics()
  di = Display()
  di.physics = ph
  di.bg_color = bg_color

  build_logo(ph)
  set_camera(di)

  di.resize(size[0], size[1], False)
  if fout is not None:
    di.update()
    di.update()  # first frame may be broken on Linux
    di.screenshot(fout)
  else:
    di.run()

