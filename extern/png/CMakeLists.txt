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
target_include_directories(png PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(png PUBLIC zlib)
cge_extern_defaults(png)

INSTALL(TARGETS png ARCHIVE DESTINATION Engine/lib)
