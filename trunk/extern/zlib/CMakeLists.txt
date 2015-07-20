IF(${OS} STREQUAL "wince")
ADD_DEFINITIONS(
	-DNO_ERRNO_H
)
ENDIF(${OS} STREQUAL "wince")

IF(${OS} STREQUAL "windows" OR "wince")
  SET(ZLIB_ADD_SOURCES iowin32.c)
ENDIF(${OS} STREQUAL "windows" OR "wince")
  
ADD_LIBRARY(zlib STATIC
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzio.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	ioapi.c
	mztools.c
	trees.c
	uncompr.c
	unzip.c
	zip.c
	zutil.c
  ${ZLIB_ADD_SOURCES}

	crc32.h
	crypt.h
	deflate.h
	inffast.h
	inffixed.h
	inflate.h
	inftrees.h
	ioapi.h
	iowin32.h
	mztools.h
	trees.h
	unzip.h
	zconf.h
	zconf.in.h
	zip.h
	zlib.h
	zutil.h
)

INSTALL(TARGETS zlib ARCHIVE DESTINATION Engine/lib)
