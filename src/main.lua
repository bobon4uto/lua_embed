print('hi')
local mylib = require("mylib")

local p = mylib.new(10, 20)

print(p:get_x()) -- 10
print(p:get_y()) -- 20

p:move(5, -3)

print(p:get_x()) -- 15
print(p:get_y()) -- 17
print(p.x)
p.x = 10
print(p.x)

print(p)
