INCLUDE_DIRECTORIES(../zlib)

ADD_LIBRARY(png STATIC
	png.c
	png.h
	pngconf.h
	pngdebug.h
	pngerror.c
	pngget.c
	pnginfo.h
	pnglibconf.h
	pngmem.c
	pngpread.c
	pngpriv.h
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngstruct.h
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c
)

INSTALL(TARGETS png ARCHIVE DESTINATION Engine/lib)
