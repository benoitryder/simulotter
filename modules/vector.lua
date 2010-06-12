
local vec3 = {
  dot = function(v1,v2) return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z end
}

local vec3_mt = {
  __eq = function(v,v2) return v.x==v2.x and v.y==v2.y and v.z==v2.z end,
  __add = function(v,v2) return vec3(v.x+v2.x, v.y+v2.y, v.z+v2.z) end,
  __sub = function(v,v2) return vec3(v.x-v2.x, v.y-v2.y, v.z-v2.z) end,
  __mul = function(v,k) return vec3(v.x*k, v.y*k, v.z*k) end,
  __unm = function(v) return vec3(-v.x,-v.y,-v.z) end,
  length2 = function(v) return v.x*v.x+v.y*v.y+v.z*v.z end,
  length = function(v) return math.sqrt(v.x*v.x+v.y*v.y+v.z*v.z) end,
  xy = function(v) return v.x,v.y end,
  xyz = function(v) return v.x,v.y,v.z end,
  __tostring = function(v) return string.format("(%f,%f,%f)", v.x, v.y, v.z) end,
  dup = function(v) return vec3(v.x,v.y,v.z) end,
}
vec3_mt.__index = vec3_mt

setmetatable(vec3, {
  __call = function(cls, x,y,z)
    local v = { x= x or 0, y= y or 0, z= z or 0 }
    return setmetatable(v, vec3_mt)
  end,
})


local vec2 = {
  dot = function(v1,v2) return v1.x*v2.x+v1.y*v2.y end
}
local vec2_mt = {
  __eq = function(v,v2) return v.x==v2.x and v.y==v2.y end,
  __add = function(v,v2) return vec2(v.x+v2.x, v.y+v2.y) end,
  __sub = function(v,v2) return vec2(v.x-v2.x, v.y-v2.y) end,
  __mul = function(v,k) return vec2(v.x*k, v.y*k) end,
  __unm = function(v) return vec2(-v.x,-v.y) end,
  length2 = function(v) return v.x*v.x+v.y*v.y end,
  length = function(v) return math.sqrt(v.x*v.x+v.y*v.y) end,
  xy = function(v) return v.x,v.y end,
  __tostring = function(v) return string.format("(%f,%f)", v.x, v.y) end,
  dup = function(v) return vec2(v.x,v.y) end,
}
vec2_mt.__index = vec2_mt

setmetatable(vec2, {
  __call = function(cls, x,y)
    local v = { x= x or 0, y= y or 0 }
    return setmetatable(v, vec2_mt)
  end,
})


vector = {
  vec3 = vec3,
  vec2 = vec2,
}
module('vector')

