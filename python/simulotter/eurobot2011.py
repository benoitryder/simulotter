
"""
Eurobot 2011: Chess'Up!
"""

from _simulotter._eurobot2011 import *
from _simulotter._eurobot2011 import _MagnetPawn
import _simulotter as _so
import eurobot as _eb
import math as _math

from _simulotter import vec2 as _vec2, vec3 as _vec3
from eurobot import WALL_WIDTH, WALL_HEIGHT

TABLE_SIZE = OGround.SIZE
TEAM_COLORS = (_eb.RAL[3020], _eb.RAL[5017])

SQUARE_SIZE = OGround.SQUARE_SIZE

# Y offset used for elements in the dispensing zone
DISPENSING_DY = 0.280


# Random configurations values.
# Values are pairs of king and queen positions. 0 is top, 4 is bottom.
RANDOM_POS = tuple( (i,j) for i in range(5) for j in range(5) if i != j )


class OPawn(_MagnetPawn):
  _shape = _so.ShCylinderZ(_vec3(_MagnetPawn.RADIUS, _MagnetPawn.RADIUS, _MagnetPawn.HEIGHT/2))
  _mass = 0.3  # 200g to 500g

  def __init__(self):
    _MagnetPawn.__init__(self, self._shape, self._mass)
    self.color = _eb.RAL[1023]


class OKing(OPawn):
  _sh_cross_bar = _so.ShBox(_vec3(0.06,0.02,0.02))
  _shape = _so.ShCompound((
      (OPawn._shape, _so.trans()),
      (_sh_cross_bar, _so.trans(_vec3(z=0.06+_MagnetPawn.HEIGHT/2))),
      (_sh_cross_bar, _so.trans(_so.matrix3(pitch=_math.pi/2), _vec3(z=0.06+_MagnetPawn.HEIGHT/2))),
      ))
  _mass = 0.5  # 300g to 700g

class OQueen(OPawn):
  _sh_figure = _so.ShSphere(0.16/2)
  _shape = _so.ShCompound((
      (OPawn._shape, _so.trans()),
      (_sh_figure, _so.trans(_vec3(z=0.16/2-0.02)))
      ))
  _mass = 0.5  # 300g to 700g



class Match(_eb.Match):
  """
  Gather match data.

  Attributes:
    pawns -- list of pawns
    kings -- pair of kings
    queens -- pair of queens
    ground -- OGround instance

  Field configuration:
    kingqueen: 0 to 19
    line1: 0 to 19
    line2: 0 to 19

  """

  class Conf:
    def __init__(self, kq, l1, l2):
      l = [kq,l1,l2]
      assert len(set(l)) == 3  # no duplicates
      for x in l:
        assert 0 <= x <= 19
      self.kingqueen, self.line1, self.line2 = kq, l1, l2

    @classmethod
    def random(cls):
      import random
      cards = range(20)
      random.shuffle(cards)
      return cls(*cards[0:3])


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
    color = _eb.RAL[9017]
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

    # Starting areas, inside borders
    sh = _so.ShBox(_vec3(ground.start_size, WALL_WIDTH, WALL_HEIGHT)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-TABLE_SIZE.x+ground.start_size, TABLE_SIZE.y-WALL_WIDTH-2*ground.start_size, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[0]
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(TABLE_SIZE.x-ground.start_size, TABLE_SIZE.y-WALL_WIDTH-2*ground.start_size, WALL_HEIGHT)/2
    o.color = TEAM_COLORS[1]

    # Secured zones, borders (centered on surrounded squares)
    sh_wall = _so.ShBox(_vec3(WALL_WIDTH, 0.150, WALL_HEIGHT)/2)
    sh_block = _so.ShBox(_vec3(0.700, 0.120, WALL_HEIGHT)/2)
    sh = _so.ShCompound((
      (sh_wall,  _so.trans(_vec2(-SQUARE_SIZE+WALL_WIDTH/2))),
      (sh_wall,  _so.trans(_vec2( SQUARE_SIZE-WALL_WIDTH/2))),
      (sh_block, _so.trans(_vec2(0, (-SQUARE_SIZE+0.120)/2))),
      ))
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(-2*SQUARE_SIZE, -2.5*SQUARE_SIZE, WALL_HEIGHT/2)
    o.color = color
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.pos = _vec3(2*SQUARE_SIZE, -2.5*SQUARE_SIZE, WALL_HEIGHT/2)
    o.color = color


    # Pawns on the field

    self.pawns = [ self.addPiece(0,0) ]
    for j in RANDOM_POS[self.conf.line1]:
      y = (j-2)*SQUARE_SIZE
      self.pawns.append( self.addPiece(-2*SQUARE_SIZE,y) )
      self.pawns.append( self.addPiece( 2*SQUARE_SIZE,y) )
    for j in RANDOM_POS[self.conf.line2]:
      y = (j-2)*SQUARE_SIZE
      self.pawns.append( self.addPiece(-1*SQUARE_SIZE,y) )
      self.pawns.append( self.addPiece( 1*SQUARE_SIZE,y) )

    # Pieces in dispensing zones

    pos_king, pos_queen = RANDOM_POS[self.conf.kingqueen]
    x = (TABLE_SIZE.x - ground.start_size)/2
    for j in range(5):
      y = -TABLE_SIZE.y/2 + (5-j)*DISPENSING_DY
      if j == pos_king:
        self.kings = ( self.addPiece(-x, y, OKing), self.addPiece(x, y, OKing) )
      elif j == pos_queen:
        self.queens = ( self.addPiece(-x, y, OQueen), self.addPiece(x, y, OQueen) )
      else:
        self.pawns.append( self.addPiece(-x,y) )
        self.pawns.append( self.addPiece( x,y) )


  def addPiece(self, x, y, cls=OPawn):
    o = cls()
    o.addToWorld(self.physics)
    o.pos = _vec2(x,y)
    return o

