#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int luaopen_example_lib(lua_State* L);
//#include "example.h"

int main(int argc, char const* argv[])
{
  lua_State * L = luaL_newstate();
  luaL_requiref(L, "example", luaopen_example_lib, 1);
  lua_pop(L, 1);
  luaL_openlibs(L);

  const char *script = argc > 1 ? argv[1] : "main.lua";
  if (luaL_dofile(L, script) != LUA_OK) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
      lua_close(L);
      return 1;
  }

  lua_close(L);
  return 0;
}
