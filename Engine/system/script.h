#ifndef SCRIPT_H
#define SCRIPT_H

#include <cge_export.h>

extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>


using std::string;


namespace CGE{

//! Provides a simple interface to the LUA scripting language
/*! Functions to call LUA from C and C-Functions to call from LUA
 * are provided here
 */
class CGE_API Script{
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
  bool getBoolSetting(const string& name, bool* exists = NULL);
  //! get a setting as string
  //! a string from engine.ini, empty (with a warning if warnIfMissing) if it isn't set
  string getStringSetting(const string& name, bool warnIfMissing = true);
  //! get a setting as float
  float getRealSetting(const string& name);
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

