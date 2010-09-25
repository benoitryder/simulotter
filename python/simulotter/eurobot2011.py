
"""
Eurobot 2011: Chess'Up!
"""

from _simulotter._eurobot2011 import *
from eurobot import TABLE_SIZE, WALL_WIDTH, WALL_HEIGHT, beacon_pos
from eurobot import RAL as _RAL

import _simulotter as _so
from _simulotter import vec2 as _vec2, vec3 as _vec3
from random import randint as _randint


CASE_SIZE = OGround.CASE_SIZE

team_colors = (_RAL[3020], _RAL[5017])


class Match:
  """
  Gather match data.

  Attributes:
    physics -- Physics instance

  """

  def __init__(self, ph=None):
    if ph is None:
      ph = _so.Physics()
    self.physics = ph

  def start(self, fconf=None):

    ph = self.physics

    # Ground and raised zone
    o = OGround()
    o.addToWorld(ph)

    # Walls (N, S, E, W)
    color = _RAL[9017]
    sh = _so.ShBox(_vec3(TABLE_SIZE.x+WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.color = color

    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, -TABLE_SIZE.y-WALL_WIDTH, WALL_HEIGHT)/2
    o.color = color

    sh = _so.ShBox(_vec3(WALL_WIDTH, TABLE_SIZE.y+2*WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x+WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = color

    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x-WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = color

    # Starting areas, inside borders (x<0, x>0)
    sh = _so.ShBox(_vec3(OGround.START_SIZE, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x+OGround.START_SIZE, TABLE_SIZE.y-WALL_WIDTH-2*OGround.START_SIZE, WALL_HEIGHT)/2
    o.color = color
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x-OGround.START_SIZE, TABLE_SIZE.y-WALL_WIDTH-2*OGround.START_SIZE, WALL_HEIGHT)/2
    o.color = color

    # Secured zones, borders (centered on surrounded cases)
    sh_wall = _so.ShBox(_vec3(WALL_WIDTH, 0.150, WALL_HEIGHT)/2)
    sh_block = _so.ShBox(_vec3(0.700, 0.120, WALL_HEIGHT)/2)
    sh = _so.ShCompound((
      (sh_wall,  _so.trans(_vec2(-CASE_SIZE+WALL_WIDTH/2))),
      (sh_wall,  _so.trans(_vec2( CASE_SIZE-WALL_WIDTH/2))),
      (sh_block, _so.trans(_vec2(0, (-CASE_SIZE+0.120)/2))),
      ))
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-2*CASE_SIZE, -2.5*CASE_SIZE, WALL_HEIGHT/2)
    o.color = color
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(2*CASE_SIZE, -2.5*CASE_SIZE, WALL_HEIGHT/2)
    o.color = color

