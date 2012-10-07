
"""

Common Eurobot definitions.

"""

WALL_WIDTH = 0.022
WALL_HEIGHT = 0.070


# predefined colors
from _simulotter import Color as _Color
RAL = {
    1013: _Color(0xff,0xf5,0xe3),
    1023: _Color(0xfc,0xbd,0x1f),
    3001: _Color(0xa3,0x17,0x1a),
    3015: _Color(0xe8,0x9c,0xb5),
    3020: _Color(0xc7,0x17,0x12),
    4008: _Color(0x7d,0x1f,0x7a),
    5005: _Color(0x00,0x2e,0x7a),
    5012: _Color(0x29,0x73,0xb8),
    5015: _Color(0x17,0x61,0xab),
    5017: _Color(0x00,0x3b,0x80),
    6018: _Color(0x4f,0xa8,0x33),
    6024: _Color(0x24,0x91,0x40),
    8002: _Color(0x6e,0x3b,0x3a),
    8017: _Color(0x2e,0x1c,0x1c),
    9005: _Color(0x03,0x05,0x0a),
    9016: _Color(0xfc,0xff,0xff),
    9017: _Color(0x14,0x17,0x1c),
    }
del _Color

TEAM_COLORS = (None, None)


class Match:
  """
  Gather match data.

  Attributes:
    physics -- Physics instance
    conf -- field configuration

  """

  def __init__(self, ph=None):
    if ph is None:
      ph = _so.Physics()
    self.physics = ph
    self.conf = None

  def prepare(self, fconf=None):
    """Add game elements."""
    raise NotImplemented


