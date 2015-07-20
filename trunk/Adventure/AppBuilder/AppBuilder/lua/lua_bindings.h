#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

extern "C"{
#include <lua.h>
#include <lauxlib.h>

void cge_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

#define cge_newlib(L,l)	(luaL_newlibtable(L,l), cge_setfuncs(L,l,0))

}

#endif
