#include "cge_io.h"
#include "lua_bindings.h"

#include <Engine/io/ZipWriter.h>
#include <Engine/io/BinFileReader.h>
#include <Engine/system/file.h>

static int createDir(lua_State* L){
  const char* dir = luaL_checkstring(L, 1);
  bool ret = CGE::Filesystem::createDir(dir);
  lua_pushboolean(L, ret);
  return 1;
}

static int removeDir(lua_State* L){
  const char* dir = luaL_checkstring(L, 1);
  bool ret = CGE::Filesystem::removeDir(dir, true);
  lua_pushboolean(L, ret);
  return 1;
}

static int packDir(lua_State* L){
  const char* zipfile = luaL_checkstring(L, 1);
  int args = lua_gettop(L) - 1;
  CGE::ZipWriter writer(zipfile);
  if (!writer.isWorking()){
    lua_pushboolean(L, 0);
    return 1;
  }
  for (int i = 0; i < args; ++i){
    const char* dir = luaL_checkstring(L, i + 2);
    std::vector<std::string> files = CGE::Filesystem::getFiles(dir);
    for (size_t i = 0; i < files.size(); ++i){
      std::string file = CGE::Filesystem::combinePath(dir, files[i]);
      CGE::BinFileReader reader(file);
      writer.addEntry(files[i], reader);
    }
  }
  lua_pushboolean(L, 1);
  return 1;
}

static int packFiles(lua_State* L){
  const char* zipfile = luaL_checkstring(L, 1);
  int args = lua_gettop(L) - 1;
  CGE::ZipWriter writer(zipfile);
  if (!writer.isWorking()){
    lua_pushfstring(L, "Cannot create zip file %s", zipfile);
    lua_error(L);
    lua_pushboolean(L, 0);
    return 1;
  }
  for (int i = 0; i < args; ++i){
    if (lua_isstring(L, i + 2)){
      const char* file = lua_tostring(L, i + 2);
      CGE::BinFileReader reader(file);
      std::string name = CGE::Filesystem::getFileComponent(file);
      writer.addEntry(name, reader);
    }
    else if (lua_istable(L, i + 2)){
      lua_pushnil(L);
      while (lua_next(L, -2)){
        const char* file = luaL_checkstring(L, -1);
        CGE::BinFileReader reader(file);
        std::string name = CGE::Filesystem::getFileComponent(file);
        writer.addEntry(name, reader);
        lua_pop(L, 1);
      }
    }
    else
      luaL_argerror(L, i + 2, "string or table expected");
  }
  lua_pushboolean(L, 1);
  return 1;
}

const struct luaL_Reg iomodule[] = {
  { "createDir", createDir },
  { "packDir", packDir },
  { "packFiles", packFiles },
  { "removeDir", removeDir },
  { NULL, NULL }
};

void createIOModule(lua_State* L){
  cge_newlib(L, iomodule);
}


