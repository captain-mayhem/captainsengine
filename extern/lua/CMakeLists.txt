IF(${OS} STREQUAL "wince")
INCLUDE_DIRECTORIES(errno)
ENDIF(${OS} STREQUAL "wince")

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

ADD_EXECUTABLE(lua_interpreter
lua.c
)

TARGET_LINK_LIBRARIES(lua_interpreter lua lualib)

ADD_EXECUTABLE(luac
luac.c
)

TARGET_LINK_LIBRARIES(luac lua lualib)

IF(${OS} STREQUAL "wince")
ADD_DEFINITIONS(-DUSE_TMPNAME=0)
ENDIF(${OS} STREQUAL "wince")

INSTALL(TARGETS lua lualib ARCHIVE DESTINATION Engine/lib)