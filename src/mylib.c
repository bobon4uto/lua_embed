#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "mylib.h"

#define POINT_MT "my_point"

typedef struct {
    double x;
    double y;
} Point;

/* Check and retrieve a Point userdata */
static Point *check_point(lua_State *L, int index)
{
    return (Point *)luaL_checkudata(L, index, POINT_MT);
}

/* Constructor: Point.new(x, y) */
static int point_new(lua_State *L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);

    Point *p = (Point *)lua_newuserdatauv(L, sizeof(Point), 0);

    p->x = x;
    p->y = y;

    luaL_setmetatable(L, POINT_MT);
    return 1;
}

/* Method: point:move(dx, dy) */
static int point_move(lua_State *L)
{
    Point *p = check_point(L, 1);

    double dx = luaL_checknumber(L, 2);
    double dy = luaL_checknumber(L, 3);

    p->x += dx;
    p->y += dy;

    return 0;
}

/* Method: point:get_x() */
static int point_get_x(lua_State *L)
{
    Point *p = check_point(L, 1);

    lua_pushnumber(L, p->x);
    return 1;
}

/* Method: point:get_y() */
static int point_get_y(lua_State *L)
{
    Point *p = check_point(L, 1);

    lua_pushnumber(L, p->y);
    return 1;
}

/* Optional destructor */
static int point_gc(lua_State *L)
{
    Point *p = check_point(L, -1);

    /*
     * If Point contained dynamically allocated data,
     * free it here.
     */

    (void)p;
    return 0;
}


#include <string.h>
static int point_index(lua_State *L)
{
    Point *p = check_point(L, 1);
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0) {
        lua_pushnumber(L, p->x);
        return 1;
    }

    if (strcmp(key, "y") == 0) {
        lua_pushnumber(L, p->y);
        return 1;
    }

    /*
     * Look up methods in the metatable.
     */
    luaL_getmetatable(L, POINT_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    return 1;
}

static int point_newindex(lua_State *L)
{
    Point *p = check_point(L, 1);
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0) {
        p->x = luaL_checknumber(L, 3);
        return 0;
    }

    if (strcmp(key, "y") == 0) {
        p->y = luaL_checknumber(L, 3);
        return 0;
    }

    return luaL_error(L, "unknown Point field '%s'", key);
}





/* Methods available on Point objects */
static const luaL_Reg point_methods[] = {
    {"move",  point_move},
    {"get_x", point_get_x},
    {"get_y", point_get_y},
    {"__gc",  point_gc},
    {NULL, NULL}
};

/* Module functions */
static const luaL_Reg module_functions[] = {
    {"new", point_new},
    {NULL, NULL}
};






/* Lua module entry point */
int luaopen_mylib(lua_State *L)
{
    /*
     * Create the Point metatable.
     */
    luaL_newmetatable(L, POINT_MT);

    /*
     * metatable.__index = metatable
     *
     * This makes point:move() find point_move.
     */

    lua_pushcfunction(L, point_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, point_newindex);
    lua_setfield(L, -2, "__newindex");

    /*
     * Register Point methods in the metatable.
     */
    luaL_setfuncs(L, point_methods, 0);

    /*
     * Remove the metatable from the stack.
     */
    lua_pop(L, 1);

    /*
     * Create and return the module table.
     */
    luaL_newlib(L, module_functions);

    return 1;
}
