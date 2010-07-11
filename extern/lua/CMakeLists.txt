IF(${OS} STREQUAL "wince")
INCLUDE_DIRECTORIES(errno)
ENDIF(${OS} STREQUAL "wince")

ADD_LIBRARY(lua STATIC
lapi.c
lcode.c
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
ltests.c
ltm.c
lundump.c
lvm.c
lzio.c
)

ADD_LIBRARY(lualib STATIC
lauxlib.c
lbaselib.c
ldblib.c
liolib.c
lmathlib.c
ltablib.c
lstrlib.c
loadlib.c
)

IF(${OS} STREQUAL "wince")
ADD_DEFINITIONS(-DUSE_TMPNAME=0)
ENDIF(${OS} STREQUAL "wince")

INSTALL(TARGETS lua lualib ARCHIVE DESTINATION Engine/lib)