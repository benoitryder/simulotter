
"""

Common Eurobot definitions.

"""

from _simulotter import vec2 as _vec2, vec3 as _vec3

TABLE_SIZE = _vec2(3.0, 2.1)
WALL_WIDTH = 0.022
WALL_HEIGHT = 0.070

def beacon_pos(team, y):
  """Return beacon coordinates.
  team is 0 or 1, y is the beacon position (-1, 0 or 1)
  """
  if team not in (0, 1):
    raise ValueError("invalid team")
  if y not in (-1, 0, 1):
    raise ValueError("invalid beacon position")
  x = team == 1 and -1 or 1
  return _vec3(
      (y==0 and x or -x) * TABLE_SIZE.x/2 + WALL_WIDTH,
      y * TABLE_SIZE.y/2 + WALL_HEIGHT,
      0.350
      )


# predefined colors
from _simulotter import Color4 as _Color4
RAL = {
    6018: _Color4(0x4f,0xa8,0x33),
    3020: _Color4(0xc7,0x17,0x12),
    5015: _Color4(0x17,0x61,0xab),
    8017: _Color4(0x2e,0x1c,0x1c),
    1013: _Color4(0xff,0xf5,0xe3),
    5005: _Color4(0x00,0x2e,0x7a),
    1023: _Color4(0xfc,0xbd,0x1f),
    9017: _Color4(0x14,0x17,0x1c),
    5017: _Color4(0x00,0x3b,0x80),
    6024: _Color4(0x24,0x91,0x40),
    }
del _Color4

