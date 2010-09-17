
"""
Eurobot 2010: Feed the World.
"""

from _simulotter._eurobot2010 import *
from eurobot import TABLE_SIZE, WALL_WIDTH, WALL_HEIGHT, beacon_pos
from eurobot import RAL as _RAL

import _simulotter as _so
from _simulotter import vec2 as _vec2, vec3 as _vec3
import math as _math
from random import randint as _randint

from _simulotter import Galipeur


# team colors
team_colors = (_RAL[5005], _RAL[1023])

# field contants
FIELD_POS_DXY = _vec2(0.450, 0.250)
FIELD_POS_Y0 = 0.128


def field_pos(x,y):
  """Compute field element position.
  (0,0) is the middle bottom.
  """
  return _vec2(x*FIELD_POS_DXY.x, y*FIELD_POS_DXY.y-TABLE_SIZE.y/2+FIELD_POS_Y0)


# field positions of tomatoes and corns
TOMATOES_FPOS = [ (0,1), (0,3) ]
CORNS_FPOS = [ (0,0), (0,2) ]
for i in range(1,4):
  TOMATOES_FPOS.extend(( (i,i-1), (i,i+1), (-i,i-1), (-i,i+1) ))
  for j in range(i+2,-1,-2):
    CORNS_FPOS.extend(( (i,j), (-i,j) ))
del i, j

# convert list of (x,y) pairs to sets of vectors
TOMATOES_FPOS = frozenset(_vec2(*x) for x in TOMATOES_FPOS)
CORNS_FPOS = frozenset(_vec2(*x) for x in CORNS_FPOS)

# field positions of fake corns, indexed by conf center/side part
FAKES_FPOS_SIDE = (
  ( (2,2),(3,3),(-2,2),(-3,3) ),
  ( (2,2),(3,5),(-2,2),(-3,5) ),
  ( (2,2),(3,1),(-2,2),(-3,1) ),
  ( (2,4),(3,1),(-2,4),(-3,1) ),
  ( (2,4),(3,5),(-2,4),(-3,5) ),
  ( (2,4),(3,3),(-2,4),(-3,3) ),
  ( (1,3),(3,3),(-1,3),(-3,3) ),
  ( (1,3),(3,5),(-1,3),(-3,5) ),
  ( (1,3),(3,1),(-1,3),(-3,1) ),
)
FAKES_FPOS_CENTER = (
  ( (0,2),(2,0),(-2,0) ),
  ( (0,2),(1,1),(-1,1) ),
  ( (0,0),(2,0),(-2,0) ),
  ( (0,0),(1,1),(-1,1) ),
)
FAKES_FPOS_SIDE   = tuple( frozenset(_vec2(*x) for x in l) for l in FAKES_FPOS_SIDE )
FAKES_FPOS_CENTER = tuple( frozenset(_vec2(*x) for x in l) for l in FAKES_FPOS_CENTER )



class OGround(_so.OGround):
  def __init__(self):
    _so.OGround.__init__(self, _RAL[6018], *team_colors)


class OBranch(_so.OSimple):

  HEIGHT = 0.25
  _shape = _so.ShCylinderZ(_vec3(0.025, 0.025, HEIGHT/2))

  def __init__(self, h):
    _so.OSimple.__init__(self)
    self.setShape(self._shape)
    #actual color differs from rules
    #self.color = _so.Color4.white()
    self.color = _so.Color4.plexi()
    self.h = h

  def createOrange(self):
    o = OOrange()
    o.addToWorld(self.physics)
    o.setMass(0)  #XXX oranges not simulated
    #XXX references
    p = self.pos
    o.pos = _vec3(p.x, p.y,
        ORaisedZone.HEIGHT+self.h+0.050*_math.cos(_math.asin(0.025/0.050)))
    return o

  @classmethod
  def branchPos(cls, x, y, h):
    """Compute position of a given branch from a given tree.
    x and y are -1 or 1 and give tree position.
    """
    assert x in (1,-1) and y in (-1,1), "invalid tree position"
    assert h in (0.25, 0.20, 0.15), "invalid tree height"
    dx = {0.25: 0, 0.20: 0.055, 0.15:  0.080}[h]
    dy = {0.25: 0, 0.20: 0.075, 0.15: -0.050}[h]
    y0 = {1: 0.250-0.070-0.075, -1: -0.250+0.080+0.050}[y]
    return _vec3(
        x * (0.500/2-0.080-0.055 + dx),
        y0 + dy + TABLE_SIZE.y/2-0.250,
        ORaisedZone.HEIGHT-cls.HEIGHT/2+h
        )


class Bac:
  """Collect bac."""

  XY0 = _vec2(TABLE_SIZE.x-0.500, -TABLE_SIZE.y)/2
  SIZE = _vec2(0.500, 0.300)
  WIDTH = 0.010
  HEIGHT = 0.300

  _band_side    = _so.ShBox(_vec3(WIDTH, SIZE.y, WALL_HEIGHT)/2)
  _plexi_side   = _so.ShBox(_vec3(WIDTH, SIZE.y, HEIGHT)/2)
  _band_back    = _so.ShBox(_vec3(SIZE.x+2*WIDTH, WIDTH, WALL_HEIGHT)/2)
  _plexi_back   = _so.ShBox(_vec3(SIZE.x+2*WIDTH, WIDTH, HEIGHT)/2)
  _plexi_front  = _so.ShBox(_vec3(SIZE.x+2*WIDTH, WIDTH, HEIGHT-0.040)/2)
  _plexi_bottom = _so.ShBox(_vec3(SIZE.x+2*WIDTH, WIDTH+SIZE.y, WALL_HEIGHT)/2)

  sh_band = _so.ShCompound((
    (_band_side, _so.trans(_vec3(-SIZE.x-WIDTH, -SIZE.y, 0)/2)),
    (_band_side, _so.trans(_vec3( SIZE.x+WIDTH, -SIZE.y, 0)/2)),
    (_band_back, _so.trans(_vec3(0, -2*SIZE.y-WIDTH, 0)/2)),
    ))
  sh_plexi = _so.ShCompound((
    (_plexi_side,   _so.trans(_vec3(-SIZE.x-WIDTH, -SIZE.y, 0)/2)),
    (_plexi_side,   _so.trans(_vec3( SIZE.x+WIDTH, -SIZE.y, 0)/2)),
    (_plexi_back,   _so.trans(_vec3(0, -2*SIZE.y-WIDTH, 0)/2)),
    (_plexi_front,  _so.trans(_vec3(0, -WIDTH, -0.040)/2)),
    (_plexi_bottom, _so.trans(_vec3(0, -SIZE.y-WIDTH, -HEIGHT)/2)),
    ))

  def __init__(self, team):
    if team == 0:
      xy0 = self.XY0
    elif team == 1:
      xy0 = self.XY0 * _vec2(-1,1)
    else:
      raise ValueError("invalid team")
    self.team = int(team)

    ob = _so.OSimple()
    ob.setShape(self.sh_band)
    ob.pos = _vec3(0,0, WALL_HEIGHT/2) + xy0
    ob.color = team_colors[self.team]

    op = _so.OSimple()
    op.setShape(self.sh_plexi)
    op.pos = _vec3(0,0, -self.HEIGHT/2) + xy0
    op.color = _so.Color4.plexi()

    self.band = ob
    self.plexi = op

  def contains(self, o):
    """Return True if the object is in the bac.
    Test is based on object's center position.
    """
    d = abs(o.pos - self.plexi.pos)
    return d.x < self.SIZE.x and d.y < self.SIZE.y and d.z < self.HEIGHT

  def addToWorld(self, ph):
    self.band.addToWorld(ph)
    self.plexi.addToWorld(ph)

  def removeFromWorld(self):
    self.band.removeFromWorld()
    self.plexi.removeFromWorld()



class Match:
  """
  Gather match data.

  Attributes:
    physics -- Physics instance
    tomatoes, corns, oranges -- lists of game objects
    bacs -- created bacs (one per team)
    conf_side, conf_center -- field configuration

  """

  def __init__(self, ph=None):
    if ph is None:
      ph = _so.Physics()
    self.physics = ph
    self.tomatoes = []
    self.corns = []
    self.oranges = []
    self.bacs = []

  def start(self, fconf=None):
    """Add game elements.

    Field configuration:
      lateral: 0 to 8
      central: (0 to 3) * 16
    Can be defined as 2-digit hexadecimal value: 0x<side><center>.
    """
    if fconf is None:
      conf_side = _randint(0,8)
      conf_center = _randint(0,3)
    else:
      cond_side, conf_center = self.extractConf(fconf)
      if not 0 <= conf_side <= 8 or not 0 <= conf_center <= 3:
        raise ValueError("invalid field configuration")

    self.conf_side, self.conf_center = conf_side, conf_center

    ph = self.physics

    # Ground and raised zone
    o = OGround()
    o.addToWorld(ph)
    o = ORaisedZone()
    o.pos = _vec3(0, TABLE_SIZE.y-0.500, 0)/2
    o.addToWorld(ph)

    # Walls (N, E, W, S)
    color = _RAL[9017]
    sh = _so.ShBox(_vec3(TABLE_SIZE.x+WALL_WIDTH, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, TABLE_SIZE.y+WALL_WIDTH, WALL_HEIGHT)/2
    o.colors = color

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

    sh = _so.ShBox(_vec3(TABLE_SIZE.x-2*Bac.SIZE.x, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(0, -TABLE_SIZE.y-WALL_WIDTH, WALL_HEIGHT)/2
    o.color = color

    # Bacs
    self.bacs = ( Bac(0), Bac(1) )
    for o in self.bacs:
      o.addToWorld(ph)

    # Tomatoes
    for v in TOMATOES_FPOS:
      self.addTomato(v)

    # Corns
    fakes = FAKES_FPOS_CENTER[conf_center] | FAKES_FPOS_SIDE[conf_side]
    for v in CORNS_FPOS:
      self.addCorn(v, v in fakes)

    # Oranges and trees
    self.addTree(-1,-1)
    self.addTree(-1, 1)
    self.addTree( 1,-1)
    self.addTree( 1, 1)


  def scores(self):
    """Compute scores as a pair (one value per team)."""
    lt = [ self.isCollected(o) for o in self.tomatoes ]
    lc = [ self.isCollected(o) for o in self.corns ]
    lo = [ self.isCollected(o) for o in self.oranges ]
    return [ lt.count(t)*150 + lc.count(t)*250 + lc.count(t)*300 for t in (0,1) ]

  def isCollected(self, o):
    """Return team number if an object is in a bac, or False."""
    for b in self.bacs:
      if b.contains(o):
        return b.team
    return False


  @classmethod
  def extractConf(cls, fconf):
    """Get lateral/central random index from field configuration."""
    return fconf % 16, fconf // 16

  def addTree(self, x, y):
    """Add a tree (3 branches and their oranges).
    x and y are -1 or 1 and give tree position.
    """
    for h in (0.25, 0.20, 0.15):
      o = OBranch(h)
      o.addToWorld(self.physics)
      o.pos = OBranch.branchPos(x,y,h)
      self.oranges.append( o.createOrange() )

  def addTomato(self, fv):
    """Add a tomato."""
    o = OTomato()
    o.addToWorld(self.physics)
    o.pos = field_pos(*fv)
    self.tomatoes.append( o )

  def addCorn(self, fv, fake):
    """Add an ear of corn."""
    o = (OCornFake if fake else OCorn)()
    o.addToWorld(self.physics)
    p = field_pos(*fv)
    if fake:
      o.pos = p
    else:
      o.plant(*p)
    self.corns.append( o )

