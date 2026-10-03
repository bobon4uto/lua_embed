local example = require("example")

local sb = example.StringBuilder.new("this is SB")
print(sb)



function test_lifetime()
  print("start func")
  local nc = example.NeedClone.new()
  print(nc.name)
  nc.name = sb
  print(nc.name)
  print("end func")
end

test_lifetime()


function test_mutability(sb)
  print("fn start: ")
  print(sb)
  sb:change()
  print("fn end: ")
  print(sb)
end

print("fn before: ")
print(sb)
test_mutability(sb)
print("fn after: ")
print(sb)

print("end programm")
