ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(BEFORE .)

IF (${OS} STREQUAL windows)
ADD_DEFINITIONS(-DWINDOWS32
-DHAVE_IO_H
-DHAVE_FCNTL_H)
ENDIF (${OS} STREQUAL windows)

ADD_LIBRARY(gif STATIC
	dev2gif.c
	dgif_lib.c
	egif_lib.c
	getarg.c
	gif_err.c
	gif_font.c
	gif_hash.c
	gifalloc.c
	qprintf.c
	quantize.c
)

INSTALL(TARGETS gif ARCHIVE DESTINATION Engine/lib)