extern "C"{
#include <lua.h>
#include <lauxlib.h>

#ifdef WIN32
#define LUA_EXPORT __declspec(dllexport)
#else
#define LUA_EXPORT
#endif

static int myTest(lua_State* L){
  printf("Hello World\n");
  return 0;
}

static const struct luaL_Reg cgelib[] = {
  { "dir", myTest },
  { NULL, NULL }
};

void cge_setfuncs(lua_State *L, const luaL_Reg *l, int nup){
  //luaL_checkversion(L);
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}

#define cge_newlib(L,l)	(luaL_newlibtable(L,l), cge_setfuncs(L,l,0))

int LUA_EXPORT luaopen_cge(lua_State* L){
  cge_newlib(L, cgelib);
  return 1;
}

}


