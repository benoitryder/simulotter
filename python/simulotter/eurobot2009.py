
"""
Eurobot 2009: Temples of Atlantis.
"""

from _simulotter._eurobot2009 import *
import _simulotter as _so
import eurobot as _eb

from _simulotter import vec2 as _vec2, vec3 as _vec3
from eurobot import TABLE_SIZE, WALL_WIDTH, WALL_HEIGHT, beacon_pos


TEAM_COLORS = (_eb.RAL[6018], _eb.RAL[3020])

# field contants
COL_SPACE_XY = _vec2(0.250, 0.200)
COL_OFFSET_XY = _vec2(0.400, 0.125)
DISP_OFFSET = _vec3(0.289, 0.250, 0.045)

COL_POS = (
    (0,1,2), (0,2,5), (0,2,4), (0,2,3), (0,1,4),
    (0,1,5), (0,4,5), (0,1,3), (0,3,5), (0,3,4),
    )


class OGround(_so.OGround):
  def __init__(self):
    _so.OGround.__init__(self, _eb.RAL[5015], *TEAM_COLORS)


class Match(_eb.Match):
  """
  Gather match data.

  Attributes:
    ground -- OGround instance

  Field configuration
    col: 0 to 9
    disp: 0 or 1

  """

  class Conf:
    def __init__(self, col, disp):
      assert 0 <= col <= 9
      assert disp in (0,1)
      self.col, self.disp = col, disp

    @classmethod
    def random(cls):
      import random
      return cls(random.randint(0,9), random.randint(0,1))

  def prepare(self, fconf=None):
    if fconf is None:
      self.conf = self.Conf.random()
    else:
      self.conf = self.Conf(*fconf)

    ph = self.physics

    # Ground and raised zone
    ground = OGround()
    ground.addToWorld(ph)
    self.ground = ground

    # Walls (N, E, W, S, small SE, small SW, plexi S)
    sh = _so.ShBox(_vec3(TABLE_SIZE.x+2*WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.color = _so.Color.white
    sh = _so.ShBox(_vec3(WALL_WIDTH, TABLE_SIZE.y+2*WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x+WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = _so.Color.white
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x-WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = _so.Color.white

    sh = _so.ShBox(_vec3(WALL_WIDTH, 0.100, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(1.800+WALL_WIDTH, -TABLE_SIZE.y+0.100, WALL_HEIGHT)/2
    o.color = _so.Color.white
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-1.800-WALL_WIDTH, -TABLE_SIZE.y+0.100, WALL_HEIGHT)/2
    o.color = _so.Color.white

    sh = _so.ShBox(_vec3(1.800+WALL_WIDTH, ph.margin_epsilon, 0.250)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, -TABLE_SIZE.y/2, 0.125)
    o.color = _so.Color.plexi
    sh = _so.ShBox(_vec3(0.578+WALL_WIDTH, ph.margin_epsilon, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(2.400, -TABLE_SIZE.y, WALL_HEIGHT)/2
    o.color = _so.Color.plexi
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-2.400, -TABLE_SIZE.y, WALL_HEIGHT)/2
    o.color = _so.Color.plexi


    # Building areas
    sh = _so.ShBox(_vec3(1.800, 0.100, ph.margin_epsilon)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, 0.050-TABLE_SIZE.y/2, ph.margin_epsilon)
    o.color = _eb.RAL[8017]

    sh = _so.ShBox(_vec3(0.600, 0.100, 0.030)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, 0.050-TABLE_SIZE.y/2, 0.015)
    o.color = _eb.RAL[8017]

    sh = _so.ShCylinderZ(_vec3(0.150, 0.150, 0.060/2))
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, 0, 0.030)
    o.color = _eb.RAL[8017]


    # Random column elements
    for j in COL_POS[self.conf.col]:
      # first team
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[0]
      o.pos = COL_SPACE_XY * _vec2(j%3-2, 3-j//3) - COL_OFFSET_XY
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[0]
      o.pos = COL_SPACE_XY * _vec2(j%3-2,   j//3) - COL_OFFSET_XY
      # second team
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[1]
      o.pos = (COL_SPACE_XY * _vec2(j%3-2, 3-j//3) - COL_OFFSET_XY)*_vec2(-1,1)
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[1]
      o.pos = (COL_SPACE_XY * _vec2(j%3-2,   j//3) - COL_OFFSET_XY)*_vec2(-1,1)


    # Dispensers

    # Fixed
    od = ODispenser()
    od.addToWorld(ph)
    od.setPos( DISP_OFFSET*_vec3(-1,0,1) - TABLE_SIZE/2*_vec2(-1,1), 2 )
    for i in range(4):
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[0]
      od.fill(o, (i+1)*0.035)
    od = ODispenser()
    od.addToWorld(ph)
    od.setPos( DISP_OFFSET*_vec3(1,0,1) - TABLE_SIZE/2, 2 )
    for i in range(4):
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[1]
      od.fill(o, (i+1)*0.035)

    # Random
    ky = 1 if self.conf.disp == 0 else -1
    od = ODispenser()
    od.addToWorld(ph)
    od.setPos( _vec3(TABLE_SIZE.x-WALL_WIDTH, DISP_OFFSET.y, DISP_OFFSET.z)*_vec3(.5,ky,1), 1 )
    for i in range(4):
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[0]
      od.fill(o, (i+1)*0.035)
    od = ODispenser()
    od.addToWorld(ph)
    od.setPos( _vec3(TABLE_SIZE.x-WALL_WIDTH, DISP_OFFSET.y, DISP_OFFSET.z)*_vec3(-.5,ky,1), 3 )
    for i in range(4):
      o = OColElem()
      o.addToWorld(ph)
      o.color = TEAM_COLORS[1]
      od.fill(o, (i+1)*0.035)


    # Lintels and lintel storages
    ols = OLintelStorage()
    ols.addToWorld(ph)
    ols.setPos(-0.200, 0)
    ol = OLintel()
    ol.addToWorld(ph)
    ol.color = TEAM_COLORS[0]
    ols.fill(ol)

    ols = OLintelStorage()
    ols.addToWorld(ph)
    ols.setPos(-0.600, 0)
    ol = OLintel()
    ol.addToWorld(ph)
    ol.color = TEAM_COLORS[0]
    ols.fill(ol)

    ols = OLintelStorage()
    ols.addToWorld(ph)
    ols.setPos(0.200, 0)
    ol = OLintel()
    ol.addToWorld(ph)
    ol.color = TEAM_COLORS[1]
    ols.fill(ol)

    ols = OLintelStorage()
    ols.addToWorld(ph)
    ols.setPos(0.600, 0)
    ol = OLintel()
    ol.addToWorld(ph)
    ol.color = TEAM_COLORS[1]
    ols.fill(ol)


