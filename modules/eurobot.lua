
-- Eurobot, comment elements
-- Constants and other things reused from year to year.

module('eurobot', package.seeall)

TABLE  = { sx = 3.0, sy = 2.1 }
WALL   = { w = 0.022, h = 0.070 }
BEACON = {
  z = 0.350,
  ox = TABLE.sx/2+WALL.w, oy = TABLE.sy/2+WALL.w
}
-- return beacon coordinates
--   t  team (1 or 2)
--   y  beacon position (-1, 0, 1)
BEACON.pos = function(t,y)
  local yy = (y<0 and -1 or y>0 and 1 or 0)
  local xx = (t == 1 and -1 or 1)
  return (yy==0 and xx or -xx)*BEACON.ox, yy*BEACON.oy, BEACON.z
end

