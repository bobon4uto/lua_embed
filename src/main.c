#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "mylib.h"

int main(int argc, char const* argv[])
{
  lua_State * L = luaL_newstate();
  luaL_requiref(L, "mylib", luaopen_mylib, 1);
  lua_pop(L, 1);
  luaL_openlibs(L);

  const char *script = argc > 1 ? argv[1] : "main.lua";
  if (luaL_dofile(L, script) != LUA_OK) {
      fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
      lua_close(L);
      return 1;
  }

  lua_close(L);
  return 0;
}
