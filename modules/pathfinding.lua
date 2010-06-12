
-- Pathfinding tools

-- Map used for A* algorithm.
-- Methods below must be defined in a subclass.
-- Note: nodes are compared using ==, thus one may define a __eq metamethod to
-- avoid having to return references to the same node objects.
local Map = class(nil)
-- Return neighbors of node n.
function Map.neighbors(self, n) error("not implemented") end
-- Return estimate of distance between two nodes.
function Map.distance_estimate(self, n1, n2) error("not implemented") end
-- Return actual distance from n1 to n2.
function Map.distance(self, n1, n2) error("not implemented") end


-- Metatable for tables with key comparaison using ==.
local eqcmp_table_mt = {
  __index = function(t,index)
    for k,v in pairs(t) do
      if k == index then return v end
    end
    return nil
  end,
  __newindex = function(t,index,val)
    for k,v in pairs(t) do
      if k == index then
        rawset(t,k,val)
      end
    end
    rawset(t,index,val)
  end,
}
local function eqcmp_table()
  return setmetatable({}, eqcmp_table_mt)
end

-- Pathfinding method (A* algorithm).
function Map.find_path(self, nfrom, ngoal)
  -- { dist. from start, estimate dist. to goal, previous node }
  local tclosed = eqcmp_table()
  local topen = eqcmp_table()
  topen[nfrom] = {0, self:distance_estimate(nfrom, ngoal), nil}
  while true do
    local n = nil
    local v = nil
    for nn,vv in pairs(topen) do
      if v == nil or vv[2] < v[2] then
        n = nn
        v = vv
      end
    end
    if n == nil then return nil end -- failed
    topen[n] = nil
    tclosed[n] = v
    if n == ngoal then
      -- goal reached, build path and return it
      local t = {}
      while n ~= nfrom do
        table.insert(t,1,n)
        n = tclosed[n][3]
      end
      return t
    end
    local neighbors = self:neighbors(n)
    for k,nn in pairs(neighbors) do
      if nn~=nil and tclosed[nn]==nil then
        local vv = {v[1]+self:distance(n,nn), self:distance_estimate(nn, ngoal), n}
        if topen[nn] == nil or vv[1] < topen[nn][1] then
          topen[nn] = vv
        end
      end
    end
  end
end


-- Map implementation using 2D vectors as nodes.
-- Graph is given as a list of edges (pairs of nodes).
-- Distances are (squared) euclidian distances (no weight applied).

require('modules/vector')
local vec2 = vector.vec2

local VectorMap = class(Map, function(self, edges)
  -- associate nodes to their neighbors
  self.nodes = eqcmp_table()
  for k,v in ipairs(edges) do
    local n1 = vec2(unpack(v[1]))
    local n2 = vec2(unpack(v[2]))
    local t

    t = self.nodes[n1]
    if t == nil then
      t = {}
      self.nodes[n1] = t
    else
      n1 = self:at(n1.x,n1.y)
    end
    t[#t+1] = n2

    t = self.nodes[n2]
    if t == nil then
      t = {}
      self.nodes[n2] = t
    else
      n2 = self:at(n2.x,n2.y)
    end
    t[#t+1] = n1
  end
end)
function VectorMap.neighbors(self, n)
  return self.nodes[n]
end
function VectorMap.distance(self, n1, n2)
  return (n2-n1):length2()
end
VectorMap.distance_estimate = VectorMap.distance

-- Return the node at a given point, if any
function VectorMap.at(self, x, y)
  local v = vec2(x,y)
  for n,_ in pairs(self.nodes) do
    if n == v then return n end
  end
  return nil
end

-- Return the nearest node from a point
function VectorMap.nearest(self, x, y)
  local d2_min = nil
  local ret = nil
  local v = vec2(x,y)
  for n,_ in pairs(self.nodes) do
    local d2 = (n-v):length2()
    if not d2_min or d2 < d2_min then
      d2_min = d2
      ret = n
    end
  end
  return ret
end


-- Modularize

pathfinding = {}
pathfinding.Map = Map
pathfinding.VectorMap = VectorMap
module('pathfinding')

