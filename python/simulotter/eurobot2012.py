
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

    # Ship borders
    W = 0.018  # border width
    deck_angle = _math.atan2(0.4-0.325, TABLE_SIZE.y-ground.start_size-W)

    sh = _so.ShBox(_vec3(0.4, W, W)/2)
    for kx in (1, -1):
      o = _so.OSimple(sh)
      o.addToWorld(ph)
      o.color = _eb.RAL[8002]
      o.pos = _vec3(kx*(TABLE_SIZE.x-0.4)/2, TABLE_SIZE.y/2-ground.start_size-W/2, W/2)

    r = 0.75
    sh = _so.ShBox(_vec3(W, r, W)/2)
    for kx in (1, -1):
      a = kx*deck_angle
      o = _so.OSimple(sh)
      o.addToWorld(ph)
      o.color = _eb.RAL[8002]
      o.trans = _so.trans(
          _so.quat(_vec3(0,0,1), a),
          # hacky offset, for better-looking result
          _vec3(kx*( TABLE_SIZE.x/2-(3*0.325+0.4)/4-W*_math.cos(a)/2 +0.002),
            (-TABLE_SIZE.y+r*_math.cos(a))/2, W/2
          ))

    # Ship's hold covers
    # static for now (cannot be opened)
    ox = (TABLE_SIZE.x-0.340)/2 + 0.018
    oy = -(TABLE_SIZE.y-0.610)/2 - 0.018
    sh_barh = _so.ShBox(_vec3(0.340, 0.018, 0.018)/2)
    sh_barv = _so.ShBox(_vec3(0.018, 0.610, 0.018)/2)
    sh = _so.ShCompound((
      (sh_barh, _so.trans(_vec3(0, 0.610-0.018, 0)/2)),
      (sh_barh, _so.trans(_vec3(0, -(0.610-0.018), 0)/2)),
      (sh_barv, _so.trans(_vec3(0.340-0.018, 0, 0)/2)),
      (sh_barv, _so.trans(_vec3(-(0.340-0.018), 0, 0)/2)),
      ))
    for kx in (1, -1):
      o = _so.OSimple(sh)
      o.addToWorld(ph)
      o.color = _eb.RAL[8002]
      o.pos = _vec3(kx*ox, oy, WALL_HEIGHT+0.018/2)

    sh = _so.ShBox(_vec3(0.340, 0.610, 0.001)/2)
    for kx in (1, -1):
      o = _so.OSimple(sh)
      o.addToWorld(ph)
      o.color = _so.Color.plexi
      o.pos = _vec3(kx*ox, oy, WALL_HEIGHT+0.018+0.001/2)

    # Palm-tree
    sh = _so.ShCylinderZ(_vec3(0.04, 0.04, 0.250)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.color = _eb.RAL[8002]
    o.pos = _vec3(0, 0, 0.250/2)
    sh = _so.ShCylinderZ(_vec3(0.150, 0.150, 0.002)/2)
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.color = _eb.RAL[6018]
    o.pos = _vec3(0, 0, 0.250+0.002/2)

    # Totems
    sh_trunk = _so.ShBox(_vec3(0.070, 0.070, 0.163)/2)
    sh_flat = _so.ShBox(_vec3(0.250, 0.250, 0.018)/2)
    sh = _so.ShCompound((
      (sh_trunk, _so.trans()),
      (sh_flat, _so.trans(_vec3(0, 0, -0.0545-0.018))),
      (sh_flat, _so.trans(_vec3(0, 0, 0))),
      (sh_flat, _so.trans(_vec3(0, 0, +0.0545+0.018))),
      ))
    for kx in (1, -1):
      o = _so.OSimple(sh)
      o.addToWorld(ph)
      o.color = _eb.RAL[8002]
      o.pos = _vec3(kx*0.8, 0, 0.163)/2

    # Treasure map
    #TODO map width is not known yet, centered on X
    map_width = 0.400
    sh_back = _so.ShBox(_vec3(map_width, 0.258, 0.010)/2)
    sh_side = _so.ShBox(_vec3(0.018, 0.258, 0.018)/2)
    # origin is the nearest point to the table
    sh = _so.ShCompound((
      (sh_back, _so.trans(_vec3(0, 0.258/2, 0.010/2))),
      (sh_side, _so.trans(_vec3(-(map_width-0.018)/2, 0.258/2, 0.010+0.018/2))),
      (sh_side, _so.trans(_vec3( (map_width-0.018)/2, 0.258/2, 0.010+0.018/2))),
      ))
    o = _so.OSimple(sh)
    o.addToWorld(ph)
    o.color = _eb.RAL[5012]
    a = _math.radians(35)
    o.trans = _so.trans(
        _so.quat(_vec3(1,0,0), a),
        _vec3(0, TABLE_SIZE.y/2, 0.315-(0.258+(0.018+0.10))*_math.sin(a))
        )

    # Bottle supports
    w = WALL_WIDTH
    sh_small = _so.ShBox(_vec3(0.089, w, w)/2)
    sh_large = _so.ShBox(_vec3(0.200, w, w)/2)
    sh = _so.ShCompound((
      (sh_small, _so.trans(_vec3(-(0.089+w)/2, -w/2, w/2))),
      (sh_small, _so.trans(_vec3( (0.089+w)/2, -w/2, w/2))),
      (sh_large, _so.trans(_vec3(0, -w/2, w+w/2))),
      (sh_large, _so.trans(_vec3(0, -w-w/2, -w/2))),
      ))
    for x in (TABLE_SIZE.x/2-0.640, -(TABLE_SIZE.x/2-0.640-0.477)):
      for kx in (1, -1):
        o = _so.OSimple(sh)
        o.addToWorld(ph)
        o.color = _eb.RAL[5012]
        o.pos = _vec3(kx*x, -TABLE_SIZE.y/2, WALL_HEIGHT)

    # Bottle buttons
    # origin is the attache point on the border (y=WALL_HEIGHT)
    # reuse sh_large and w from bottle supports
    #TODO sh_buttonv Y size is not known
    sh_buttonh = _so.ShBox(_vec3(w, 2*w, w)/2)
    sh_buttonv = _so.ShBox(_vec3(w, w, 0.103)/2)
    sh = _so.ShCompound((
      (sh_large, _so.trans(_vec3(0, -w-w/2, w/2))),
      (sh_buttonh, _so.trans(_vec3(0, 0, w/2))),
      (sh_buttonv, _so.trans(_vec3(0, w+w/2, -WALL_HEIGHT+0.003+0.103/2))),
      ))
    for x in (TABLE_SIZE.x/2-0.640, -(TABLE_SIZE.x/2-0.640-0.477)):
      for kx in (1, -1):
        o = _so.OSimple(sh, 0.100)
        o.addToWorld(ph)
        o.color = TEAM_COLORS[ 1 if kx == 1 else 0 ]
        o.pos = _vec3(kx*x, -TABLE_SIZE.y/2, WALL_HEIGHT)


    # Bullions
    bsize = OBullion.SIZE
    l = []  # x, y, z_bottom, angle_z
    l.append((0, -TABLE_SIZE.y/2+0.647, 0, 0))
    for kx in (1, -1):
      # bullion along the deck
      v = _vec2(TABLE_SIZE.x/2-0.400-bsize.y/2, TABLE_SIZE.y/2-ground.start_size)
      v += _vec2(0, -0.285-bsize.x/2).rotate(deck_angle)
      l.append((kx*v.x, v.y, 0, _math.pi/2+kx*deck_angle))
      # bullions on totems
      l.append((kx*0.8/2,  (0.070+0.090)/2, 0.018+0.0545+0.018, 0))
      l.append((kx*0.8/2, -(0.070+0.090)/2, 0.018+0.0545+0.018, 0))
    for x,y,z0,a in l:
      o = OBullion()
      o.addToWorld(ph)
      o.trans = _so.trans(
          _so.quat(_vec3(0,0,1), a),
          _vec3(x, y, z0 + bsize.z/2 + ph.margin_epsilon)
          )

    # Coins
    #XXX exact position of several coins is not known
    l = [
        # (x>0, y, z_bottom, angle_z)
        (TABLE_SIZE.x/2-0.450, -TABLE_SIZE.y/2+0.300, 0, 0),
        (0.090, -TABLE_SIZE.y/2+0.300, 0, 0)
        ]
    for i in range(-3,4):
      a = i*_math.pi/4
      ca = _math.cos(a)
      sa = _math.sin(a)
      l.append( (0.8/2+0.25*ca, 0.25*sa, 0, a) )
      if i % 2 == 1:
        l.append( (0.8/2+0.100*ca, 0.110*sa, 0.018, a) )
        l.append( (0.8/2+0.100*ca, 0.110*sa, 0.163, a) )
    # group coins them by pairs for random picking of black coins
    lpairs = [(
        (0, -TABLE_SIZE.y/2+0.300+0.090, 0, _math.pi/2),
        (0, -TABLE_SIZE.y/2+0.300-0.090, 0, -_math.pi/2)
        )]
    for x,y,z0,a in l:
      lpairs.append(((x,y,z0,a), (-x,y,z0,-a+_math.pi)))

    # pick up random pairs which will be black
    import random
    black_pairs = random.sample(lpairs, 2)

    # last pair, never black
    lpairs.append((
      (  TABLE_SIZE.x/2-0.5-0.5,  TABLE_SIZE.y/2-0.5, 0, 0),
      (-(TABLE_SIZE.x/2-0.5-0.5), TABLE_SIZE.y/2-0.5, 0, _math.pi)
      ))

    # finally, created the coins
    for p in lpairs:
      white = p not in black_pairs
      for x,y,z0,a in p:
        o = OCoin(white)
        o.addToWorld(ph)
        o.trans = _so.trans(
            _so.quat(_vec3(0,0,1), a),
            _vec3(x, y, z0 + OCoin.CUBE_SIZE + ph.margin_epsilon)
            )

