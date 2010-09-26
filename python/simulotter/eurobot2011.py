
"""
Eurobot 2011: Chess'Up!
"""

from _simulotter._eurobot2011 import *
from eurobot import TABLE_SIZE, WALL_WIDTH, WALL_HEIGHT, beacon_pos
from eurobot import RAL as _RAL

import _simulotter as _so
from _simulotter import vec2 as _vec2, vec3 as _vec3
from random import randint as _randint


team_colors = (_RAL[3020], _RAL[5017])

CASE_SIZE = OGround.CASE_SIZE


# Random configurations values.
# Values are pairs of king and queen positions. 0 is top, 4 is bottom.
RANDOM_POS = tuple( (i,j) for i in range(5) for j in range(5) if i != j )


class OPawn(_so.OSimple):
  _shape = _so.ShCylinderZ(_vec3(0.2, 0.2, 0.05)/2)
  _mass = 0.3  # 200g to 500g

  def __init__(self):
    _so.OSimple.__init__(self, self._shape, self._mass)
    self.color = _RAL[1023]


class OKing(OPawn):
  _sh_figure = _so.ShConeZ(0.08, 0.18)
  _shape = _so.ShCompound((
      (OPawn._shape, _so.trans()),
      (_sh_figure, _so.trans(_vec3(z=0.18/2)))
      ))
  _mass = 0.5  # 300g to 700g

class OQueen(OPawn):
  _sh_figure = _so.ShSphere(0.16/2)
  _shape = _so.ShCompound((
      (OPawn._shape, _so.trans()),
      (_sh_figure, _so.trans(_vec3(z=0.16/2-0.02)))
      ))
  _mass = 0.5  # 300g to 700g



class Match:
  """
  Gather match data.

  Attributes:
    physics -- Physics instance
    conf_kq -- random positions of king and queen
    conf_l1 -- random positions on lines 1
    conf_l2 -- random positions on lines 2
    pawns -- list of pawns
    kings -- pair of kings
    queens -- pair of queens

  """

  def __init__(self, ph=None):
    if ph is None:
      ph = _so.Physics()
    self.physics = ph

  def start(self, fconf=None):
    """Add game elements.

    Field configuration:
      king and queen: 0 to 19
      line 1: 0 to 19
      line 2: 0 to 19
    Can be defined as a 3-digit hexadecimal value: 0x<k-q><line1><line2>.
    """

    if fconf is None:
      conf_kq = _randint(0,19)
      conf_l1 = _randint(0,19)
      conf_l2 = _randint(0,19)
    else:
      conf_kq, conf_l1, conf_l2 = self.extractConf(fconf)

    self.conf_kq, self.conf_l1, self.conf_l2 = conf_kq, conf_l1, conf_l2

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

    # Starting areas, inside borders
    sh = _so.ShBox(_vec3(OGround.START_SIZE, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x+OGround.START_SIZE, TABLE_SIZE.y-WALL_WIDTH-2*OGround.START_SIZE, WALL_HEIGHT)/2
    o.color = team_colors[0]
    o = _so.OSimple()
    o.setShape(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x-OGround.START_SIZE, TABLE_SIZE.y-WALL_WIDTH-2*OGround.START_SIZE, WALL_HEIGHT)/2
    o.color = team_colors[1]

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


    # Pawns on the field

    self.pawns = [ self.addPiece(0,0) ]
    for j in RANDOM_POS[self.conf_l1]:
      y = (j-2)*CASE_SIZE
      self.pawns.append( self.addPiece(-2*CASE_SIZE,y) )
      self.pawns.append( self.addPiece( 2*CASE_SIZE,y) )
    for j in RANDOM_POS[self.conf_l2]:
      y = (j-2)*CASE_SIZE
      self.pawns.append( self.addPiece(-1*CASE_SIZE,y) )
      self.pawns.append( self.addPiece( 1*CASE_SIZE,y) )

    # Pieces in dispensing zones

    pos_king, pos_queen = RANDOM_POS[self.conf_kq]
    x = (TABLE_SIZE.x - OGround.START_SIZE)/2
    for j in range(5):
      y = -TABLE_SIZE.y/2 + (5-j)*(TABLE_SIZE.y-OGround.START_SIZE-WALL_WIDTH)/6
      if j == pos_king:
        self.kings = ( self.addPiece(-x, y, OKing), self.addPiece(x, y, OKing) )
      elif j == pos_queen:
        self.queens = ( self.addPiece(-x, y, OQueen), self.addPiece(x, y, OQueen) )
      else:
        self.pawns.append( self.addPiece(-x,y) )
        self.pawns.append( self.addPiece( x,y) )


  @classmethod
  def extractConf(cls, fconf):
    """Get configuration random indexes from field configuration."""
    return fconf&0xf, (fconf>>4)&0xf, (fconf>>8)&0xf


  def addPiece(self, x, y, cls=OPawn):
    o = cls()
    o.addToWorld(self.physics)
    o.pos = _vec2(x,y)
    return o

