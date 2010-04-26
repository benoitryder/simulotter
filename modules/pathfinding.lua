
-- Pathfinding tools

module('pathfinding', package.seeall)


Node = class(nil, function(self, map, x, y)
  self.map = map
  self.x = x
  self.y = y
  self.o = OSimple()
  self.o:set_shape(map.node_shape)
  local ax, ay = map:node2xy(x,y)
  self.o:set_pos(ax, ay, map.node_z)
  self:set(1)
end)
function Node.set(self, v)
  self.v = v
  self.o:set_color(colors.gray(1-v/100))
end
function Node.show(self)
  if not self.o:is_in_world() then
    self.o:add_to_world()
  end
end
function Node.hide(self)
  if self.o:is_in_world() then
    self.o:remove_from_world()
  end
end


-- Pathfinding map.
-- The following additional methods must be defined:
--   node2xy: convert node position to absolute coordinates
--   xy2node: convert absolute coordinates to node position
-- Note: node positions may not be actual node positions.
Map = class(nil, function(self, p0, p1, step)
  self.nodes = {}
  self.p0 = p0
  self.p1 = p1
  self.step = step
end)
-- Configuration attributes (can be overriden on map instance)
--   shape: shape used for node objects
--   z pos: z-position of node objects
Map.node_shape = Shape:sphere(0.02)
Map.node_z = 1


-- Create nodes.
function Map.init(self)
  for x=self.p0[1],self.p1[1],self.step[1] do
    local tx = {}
    self.nodes[x] = tx
    for y=self.p0[2],self.p1[2],self.step[2] do tx[y] = Node(self,x,y) end
  end
end

function Map.show(self)
  for k,tx in pairs(self.nodes) do
    for kk,n in pairs(tx) do n:show() end
  end
end
function Map.hide(self)
  for k,tx in pairs(self.nodes) do
    for kk,n in pairs(tx) do n:hide() end
  end
end
function Map.node(self, x, y)
  local t = self.nodes[x]
  if not t then return nil end
  return t[y]
end

function Map.nearest_node(self, x, y)
  local nx, ny = self:xy2node(x,y)
  local stx, sty = unpack(self.step)
  return self:node( math.floor(nx/stx+stx/2)*stx, math.floor(ny/sty+sty/2)*sty )
end
function Map.node_distance(self, n0, n1)
  local x0, y0 = self:node2xy(n0.x,n0.y)
  local x1, y1 = self:node2xy(n1.x,n1.y)
  return (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)
end

-- Pathfinding method (A* algorithm).
-- Neighbors are the 8 adjacent nodes (diagonals are included).
function Map.find_path(self, nfrom, ngoal)
  local nedge = {}
  local tclosed = {}
  local topen = { [nfrom]={0, self:node_distance(nfrom, ngoal), nil} }
  while true do
    local n = nil
    local v = nil
    for nn,vv in pairs(topen) do
      if v == nil or vv[1] < v[1] then
        n = nn
        v = vv
      end
    end
    if n == nil then return nil end -- failed
    topen[n] = nil
    tclosed[n] = v
    if n == ngoal then
      local t = {}
      while n ~= nfrom do
        table.insert(t,1,n)
        n = tclosed[n][3]
      end
      return t
    end
    local neighbors = {
      {n.x-0.5,n.y-1}, {n.x,n.y-1}, {n.x+0.5,n.y-1},
      {n.x-0.5,n.y  },              {n.x+0.5,n.y  },
      {n.x+0.5,n.y+1}, {n.x,n.y+1}, {n.x+0.5,n.y+1},
    }
    for k,xy in pairs(neighbors) do
      local nn = self:node(unpack(xy))
      if nn~=nil and tclosed[nn]==nil then
        local vv = {v[1]+(nn.v+n.v)*self:node_distance(n,nn), self:node_distance(nn, ngoal), n}
        if topen[nn] == nil or vv[1] < topen[nn][1] then
          topen[nn] = vv
        end
      end
    end
  end
end


