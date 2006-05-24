
#include "engine.h"
#include "script.h"

namespace Engine{

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
  Log << "Initialize Lua Scripting environment\n";
	L = lua_open();
  luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);

	if (luaL_loadfile(L, "engine.ini") || lua_pcall(L,0,0,0)){
    Log << "Cannot load engine.ini" <<lua_tostring(L, -1);
    EXIT();
	}
}

int Script::getNumberSetting(const string& name){
  lua_getglobal(L, name.c_str());
  if (!lua_isnumber(L,-1)){
    Log << "Lua_Error: " << name << " is no number.\n";
  }
  return (int)lua_tonumber(L,-1);
}

bool Script::getBoolSetting(const string& name){
  lua_getglobal(L, name.c_str());
  if (!lua_isboolean(L,-1)){
    Log << "Lua_Error: " << name << " is no boolean.\n";
  }
  return (bool)lua_toboolean(L,-1);
}

string Script::getStringSetting(const string& name){
  lua_getglobal(L, name.c_str());
  if (!lua_isstring(L,-1)){
    Log << "Lua_Error: " << name << " is no string.\n";
  }
  return string(lua_tostring(L,-1));
}

}

