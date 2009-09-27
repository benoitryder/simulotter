local math = math
module('colors')

function i2f(r,g,b)
  return {r/255.0, g/255.0, b/255.0, 1.0}
end
function h2f(h)
  return i2f(math.floor(h/0x10000), math.floor(h/0x100)%0x100, h%0x100)
end

white = {1.0, 1.0, 1.0, 1.0}
black = {0.0, 0.0, 0.0, 1.0}
plexi = {.70, .90, .95, 0.5}

ral_6018 = h2f(0x4fa833)
ral_3020 = h2f(0xc71712)
ral_5015 = h2f(0x1761ab)
ral_8017 = h2f(0x2e1c1c)
ral_1013 = h2f(0xfff5e3)
ral_5005 = h2f(0x002e7a)
ral_1023 = h2f(0xfcbd1f)
ral_9017 = h2f(0x14171c)

