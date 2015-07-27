#include "lua_bindings.h"
#include "cge_io.h"

#include <system/engine.h>

extern "C"{
#include <lualib.h>
}

#ifdef WIN32
#define LUA_EXPORT __declspec(dllexport)
#else
#define LUA_EXPORT
#endif

void engineMain(int argc, char** argv){

}

class CommThread{
public:
  static CommThread* createSelf(lua_State* L){
    void* data = lua_newuserdata(L, sizeof(CommThread));
    return new (data)CommThread(L);
  }
  static CommThread* getSelf(lua_State* L){
    CommThread* thrd;
    lua_getfield(L, LUA_REGISTRYINDEX, "CGE_COMM_SELF");
    thrd = (CommThread*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return thrd;
  }
private:
  CommThread(lua_State *state) : L(state), channel(NULL){
    lua_setfield(L, LUA_REGISTRYINDEX, "CGE_COMM_SELF");
    thread.createSelf();
  }
  lua_State* L;
  CGE::Thread thread;
  CGE::Condition cond;
  const char* channel;
  CommThread* prev;
};

CGE::Thread engine_thread;

extern "C"{
  LUA_EXPORT int luaopen_cge(lua_State* L);
}

void engine_thread_func(void* data){
  lua_State* L = (lua_State*)data;
  luaL_openlibs(L);
  luaL_requiref(L, "cge", luaopen_cge, 1);
  lua_pop(L, 1);
  /*int args = lua_gettop(L);
  for (int i = 0; i < args; ++i){
    if (lua_istable(L, i + 1)){
      lua_pushvalue(L, i + 1);
      lua_setfield(L, LUA_REGISTRYINDEX, "cge.config");
    }
  }*/
  CGE::Engine::mainLoop(0, NULL, engineMain);
  CommThread* thread = CommThread::getSelf(L);
  thread->~CommThread();
  lua_close(L);
}

static int initEngine(lua_State* L){
  if (engine_thread.isRunning()){
    lua_pushstring(L, "Engine already running");
    lua_error(L);
  }
  lua_State* L2 = luaL_newstate();
  if (L2 == NULL)
    luaL_error(L, "cannot create new lua state");
  engine_thread.create(engine_thread_func, L2);
  /*int args = lua_gettop(L);
  lua_State* L2 = lua_newthread(L);
  lua_xmove(L, L2, args);
  engine_thread.create(engine_thread_func, L2);
  lua_thread = luaL_ref(L, LUA_REGISTRYINDEX);*/
  return 0;
}

static int shutdownEngine(lua_State* L){
  if (engine_thread.isRunning()){
    CGE::Engine::instance()->requestShutdown();
    engine_thread.join();
    CommThread* thrd = CommThread::getSelf(L);
    thrd->~CommThread();
    //luaL_unref(L, LUA_REGISTRYINDEX, lua_thread);
  }
  return 0;
}

static int messageSend(lua_State* L){
  return 0;
}

static int messageReceive(lua_State* L){
  return 0;
}

static const struct luaL_Reg cgelib[] = {
  { "init", initEngine },
  { "shutdown", shutdownEngine },
  { "send", messageSend },
  { "receive", messageReceive },
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

LUA_EXPORT int luaopen_cge(lua_State* L){
  CommThread::createSelf(L);
  cge_newlib(L, cgelib);
  createIOModule(L);
  lua_setfield(L, -2, "io");
  return 1;
}

}


