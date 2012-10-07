
"""
Eurobot 2013: Happy Birthday!
"""

from _simulotter._eurobot2013 import *
import _simulotter as _so
import eurobot as _eb
import math as _math
import random

from _simulotter import vec2 as _vec2, vec3 as _vec3
from eurobot import WALL_WIDTH, WALL_HEIGHT

TABLE_SIZE = OGround.SIZE
TEAM_COLORS = (_eb.RAL[5017], _eb.RAL[3001])


class OPlate(_so.OSimple):
  _mass = 0.220
  _sh_bar_x = _so.ShBox(_vec3(.170, .022, .022)/2)
  _sh_bar_y = _so.ShBox(_vec3(.022, .170, .022)/2)
  _shape = _so.ShCompound((
    (_sh_bar_x, _so.trans(_vec3(0, -(.170-.022), .005)/2)),
    (_sh_bar_x, _so.trans(_vec3(0,  (.170-.022), .005)/2)),
    (_sh_bar_y, _so.trans(_vec3(-(.170-.022), 0, .005)/2)),
    (_sh_bar_y, _so.trans(_vec3( (.170-.022), 0, .005)/2)),
    (_so.ShBox(_vec3(.170, .170, .005)/2), _so.trans(_vec3(0, 0, -.022/2))),
    ))

  def __init__(self):
    _so.OSimple.__init__(self, self._shape, self._mass)
    self.color = _eb.RAL[3015]


class OCherry(_so.OSimple):
  r = 0.040/2
  _mass = 0.0027
  _shape = _so.ShSphere(r)

  def __init__(self, team=None):
    _so.OSimple.__init__(self, self._shape, self._mass)
    if team is None:
      self.color = _eb.RAL[9016]
    else:
      self.color = TEAM_COLORS[team]


class Match(_eb.Match):
  """
  Gather match data.

  Attributes:
    ground -- OGround instance

  """

  def prepare(self, fconf=None):

    ph = self.physics

    # Ground and cake
    ground = OGround()
    ground.addToWorld(ph)
    self.ground = ground
    cake = OCake()
    cake.addToWorld(ph)

    # sideboards (NE, SE, NW, SE)
    sh = _so.ShBox(_vec3(OGround.SQUARE_SIZE, 0.100, WALL_WIDTH)/2)
    for x in (TABLE_SIZE.x-OGround.SQUARE_SIZE, -TABLE_SIZE.x+OGround.SQUARE_SIZE):
      for y in (TABLE_SIZE.y-0.100, -TABLE_SIZE.y+0.100):
        o = _so.OSimple(sh)
        o.addToWorld(ph)
        o.pos = _vec3(x, y, 0)/2
        o.color = _eb.RAL[9016]

    # Walls (NE, NW, S, E, W)
    sh = _so.ShBox(_vec3(TABLE_SIZE.x/2+WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x/2+WALL_WIDTH, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[1]
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x/2-WALL_WIDTH, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[0]

    sh = _so.ShBox(_vec3(TABLE_SIZE.x+2*WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, -TABLE_SIZE.y-WALL_WIDTH, WALL_HEIGHT)/2
    o.color = _eb.RAL[1023]

    sh = _so.ShBox(_vec3(WALL_WIDTH, TABLE_SIZE.y+2*WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x+WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[1]

    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x-WALL_WIDTH, 0, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[0]

    # gifts
    for x in (-0.9, -0.3, 0.3, 0.9):
      o = OGiftSupport()
      o.addToWorld(ph)
      o.pos = _vec3(x, -TABLE_SIZE.y/2-WALL_WIDTH, WALL_HEIGHT-0.060)

    # candles
    def add_candle(r, z, a, color):
      o = OCandle()
      o.addToWorld(ph)
      o.color = color
      x, y = r*_math.cos(-_math.pi/2 + a), r*_math.sin(-_math.pi/2 + a)
      o.pos = _vec3(x, y+TABLE_SIZE.y/2, z+OCandle.HEIGHT/2)

    for i in range(6):
      a = _math.radians(-7.5*(2*i+1))
      if i < 2:
        colors = (_eb.RAL[9016], _eb.RAL[9016])
      elif i == 5:
        colors = TEAM_COLORS
      else:
        colors = random.choice((TEAM_COLORS, TEAM_COLORS[::-1]))
      add_candle(0.450, 0.100, a, colors[0])
      add_candle(0.450, 0.100, -a, colors[1])

    for i in range(4):
      a = _math.radians(-11.25*(2*i+1))
      if i == 3:
        colors = TEAM_COLORS
      else:
        colors = random.choice((TEAM_COLORS, TEAM_COLORS[::-1]))
      add_candle(0.350, 0.200, a, colors[0])
      add_candle(0.350, 0.200, -a, colors[1])


    # plates and cherries
    cherries_pos = [  # position, in cherry radii
        (-2, -2), (0, -2), (2, -2),
        (-1, 0), (1, 0),
        (-2, 2), (0, 2), (2, 2),
        ]
    for kx in (-1, 1):
      for y in (-.750, -.400, 0, .400, .750):
        o = OPlate()
        o.addToWorld(ph)
        o.pos = _vec2(kx*(TABLE_SIZE.x-OGround.SQUARE_SIZE)/2, y)
        # rotten cherry (colored)
        i_rotten = random.randrange(len(cherries_pos))
        for i,(cx,cy) in enumerate(cherries_pos):
          if i == i_rotten:
            co = OCherry(1 if kx == 1 else 0)
          else:
            co = OCherry()
          co.addToWorld(ph)
          co.pos = o.pos + _vec3(cx*co.r, cy*co.r, co.r+0.010)

    # glasses
    glasses_pos = [  # positions for x>0
        (.300,  .050), (.600,  .050),
        (.150, -.200), (.450, -.200),
        (.300, -.450), (.600, -.450),
        ]
    for kx in (-1, 1):
      for x,y in glasses_pos:
        o = OGlass()
        o.addToWorld(ph)
        o.pos = _vec2(kx*x, y)


