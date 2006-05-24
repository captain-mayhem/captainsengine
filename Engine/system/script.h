#ifndef SCRIPT_H
#define SCRIPT_H

extern "C"{
#include "../include/lua.h"
#include "../include/lauxlib.h"
#include "../include/lualib.h"
}

#include <string>

#include "singleton.h"

using std::string;


namespace Engine{

//! Provides a simple interface to the LUA scripting language
/*! Functions to call LUA from C and C-Functions to call from LUA
 * are provided here
 */
class Script{
public:
	//! Destructor
	~Script();
  //! init singleton
  inline static void init() {scr_ = new Script();}
  inline static void kill(){delete scr_;}
	//! init scripting environment
	void initEnv();
  //! get a setting as integer
  int getNumberSetting(const string& name);
  //! get a setting as boolean
  bool getBoolSetting(const string& name);
  //! get a setting as string
  string getStringSetting(const string& name);
  //! get the script instance
  inline static Script* instance() {return scr_;}
	
protected:
  //! Constructor
  Script();
  //! The scripting class
  static Script* scr_;
	//! The stack for communication between LUA and C
	lua_State* L;
};

}

#endif

