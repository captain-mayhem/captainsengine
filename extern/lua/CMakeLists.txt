# Lua is compiled into the engine library and exported from it: applications
# (the Adventure, HQClient) call the Lua API directly, and Lua must exist only
# once per process. It's an object library so that all of it ends up in the
# engine, not just the functions the engine itself references.
add_library(lua OBJECT
lapi.c
lcode.c
lctype.c
ldebug.c
ldo.c
ldump.c
lfunc.c
lgc.c
llex.c
lmem.c
lobject.c
lopcodes.c
lparser.c
lstate.c
lstring.c
ltable.c
ltm.c
lundump.c
lvm.c
lzio.c
lauxlib.c
lbaselib.c
lbitlib.c
lcorolib.c
ldblib.c
liolib.c
lmathlib.c
loslib.c
lstrlib.c
ltablib.c
loadlib.c
linit.c
)
target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (WIN32)
  # with LUA_CORE/LUA_LIB (set by the Lua sources) this makes LUA_API dllexport
  target_compile_definitions(lua PRIVATE LUA_BUILD_AS_DLL)
endif()
cge_extern_defaults(lua)

if (OS STREQUAL "linux" OR OS STREQUAL "android")
  set(LUA_DEP_LIBS m)
endif()

add_executable(lua_interpreter lua.c)
target_link_libraries(lua_interpreter lua ${LUA_DEP_LIBS})

add_executable(luac luac.c)
target_link_libraries(luac lua ${LUA_DEP_LIBS})
