
#include "engine.h"
#include "script.h"
#include "file.h"

namespace CGE{

TR_CHANNEL(CGE_Script);

Script* Script::scr_ = NULL;

Script::Script() : L(0) {
}

Script::~Script(){
	if (L)
		lua_close(L);
}

//void Script::init(){
//  scr_ = new Script();
//}

void Script::initEnv(){
  TR_USE(CGE_Script);
  TR_INFO("Initialize Lua Scripting environment");
	L = luaL_newstate();
  luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);

  if (luaL_loadfile(L, (Filesystem::getCwd()+"/engine.ini").c_str()) || lua_pcall(L,0,0,0)){
    TR_WARN("Cannot load engine.ini: %s", lua_tostring(L, -1));
	}
}

int Script::getNumberSetting(const string& name){
  TR_USE(CGE_Script);
  lua_getglobal(L, name.c_str());
  if (!lua_isnumber(L,-1)){
    TR_WARN("Lua_Error: %s is no number.", name.c_str());
    lua_pop(L, 1);
    return 0;
  }
  int ret = (int)lua_tonumber(L,-1);
  lua_pop(L, 1);
  return ret;
}

bool Script::getBoolSetting(const string& name, bool* exists){
  TR_USE(CGE_Script);
  lua_getglobal(L, name.c_str());
  if (!lua_isboolean(L,-1)){
    TR_WARN("Lua_Error: %s is no boolean.", name.c_str());
    if (exists)
      *exists = false;
    lua_pop(L, 1);
    return false;
  }
  if (exists)
    *exists = true;
  bool ret = lua_toboolean(L,-1) != 0;
  lua_pop(L, 1);
  return ret;
}

string Script::getStringSetting(const string& name){
  TR_USE(CGE_Script);
  lua_getglobal(L, name.c_str());
  if (!lua_isstring(L,-1)){
    TR_WARN("Lua_Error: %s is no string.", name.c_str());
    lua_pop(L, 1);
    return "";
  }
  string ret = string(lua_tostring(L,-1));
  lua_pop(L, 1);
  return ret;
}

float Script::getRealSetting(const string& name){
  TR_USE(CGE_Script);
  lua_getglobal(L, name.c_str());
  if (!lua_isnumber(L,-1)){
    TR_WARN("Lua_Error: %s is no string.", name.c_str());
    lua_pop(L, 1);
    return 0;
  }
  float ret = (float)lua_tonumber(L,-1);
  lua_pop(L, 1);
  return ret;
}

}

