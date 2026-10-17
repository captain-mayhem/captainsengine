ADD_LIBRARY(lua STATIC
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
)

ADD_LIBRARY(lualib STATIC
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
target_link_libraries(lualib PUBLIC lua)
cge_extern_defaults(lua)
cge_extern_defaults(lualib)

ADD_EXECUTABLE(lua_interpreter
lua.c
)

IF (OS STREQUAL "linux" OR OS STREQUAL "android")
SET(LUA_DEP_LIBS m)
ENDIF()

TARGET_LINK_LIBRARIES(lua_interpreter lua lualib ${LUA_DEP_LIBS})

ADD_EXECUTABLE(luac
luac.c
)

TARGET_LINK_LIBRARIES(luac lua lualib ${LUA_DEP_LIBS})

INSTALL(TARGETS lua lualib ARCHIVE DESTINATION Engine/lib)