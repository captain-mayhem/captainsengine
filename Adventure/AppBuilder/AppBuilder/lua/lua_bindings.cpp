#include "lua_bindings.h"
#include "cge_io.h"

#include <system/engine.h>

#ifdef WIN32
#define LUA_EXPORT __declspec(dllexport)
#else
#define LUA_EXPORT
#endif

void engineMain(int argc, char** argv){

}

CGE::Thread engine_thread;
int lua_thread;

void engine_thread_func(void* data){
  lua_State* L = (lua_State*)data;
  int args = lua_gettop(L);
  for (int i = 0; i < args; ++i){
    if (lua_istable(L, i + 1)){
      lua_pushvalue(L, i + 1);
      lua_setfield(L, LUA_REGISTRYINDEX, "cge.config");
    }
  }
  CGE::Engine::mainLoop(0, NULL, engineMain);
}

static int initEngine(lua_State* L){
  if (engine_thread.isRunning()){
    lua_pushstring(L, "Engine already running");
    lua_error(L);
  }
  int args = lua_gettop(L);
  lua_State* L2 = lua_newthread(L);
  lua_xmove(L, L2, args);
  engine_thread.create(engine_thread_func, L2);
  lua_thread = luaL_ref(L, LUA_REGISTRYINDEX);
  return 0;
}

static int shutdownEngine(lua_State* L){
  if (engine_thread.isRunning()){
    CGE::Engine::instance()->requestShutdown();
    engine_thread.join();
    luaL_unref(L, LUA_REGISTRYINDEX, lua_thread);
  }
  return 0;
}

static const struct luaL_Reg cgelib[] = {
  { "init", initEngine },
  { "shutdown", shutdownEngine },
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

extern "C"{

int LUA_EXPORT luaopen_cge(lua_State* L){
  cge_newlib(L, cgelib);
  createIOModule(L);
  lua_setfield(L, -2, "io");
  return 1;
}

}


