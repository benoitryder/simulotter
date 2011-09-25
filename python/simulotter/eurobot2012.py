
"""
Eurobot 2012: Treasure Island
"""

from _simulotter._eurobot2012 import *
import _simulotter as _so
import eurobot as _eb
import math as _math

from _simulotter import vec2 as _vec2, vec3 as _vec3
from eurobot import WALL_WIDTH, WALL_HEIGHT

TABLE_SIZE = OGround.SIZE
TEAM_COLORS = (_eb.RAL[4008], _eb.RAL[3001])


class Match(_eb.Match):
  """
  Gather match data.

  Attributes:
    ground -- OGround instance

  """

  class Conf:
    def __init__(self):
      pass

    @classmethod
    def random(cls):
      return cls()


  def prepare(self, fconf=None):
    if fconf is None:
      self.conf = self.Conf.random()
    else:
      self.conf = self.Conf(*fconf)

    ph = self.physics

    # Ground
    ground = OGround()
    ground.addToWorld(ph)
    self.ground = ground

    # Walls (N, S, E, W)
    color = _eb.RAL[5012]
    sh = _so.ShBox(_vec3(TABLE_SIZE.x+WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.color = color

    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, -TABLE_SIZE.y-WALL_WIDTH, WALL_HEIGHT)/2
    o.color = color

    sh = _so.ShBox(_vec3(WALL_WIDTH, TABLE_SIZE.y+2*WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x+WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = color

    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x-WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = color


