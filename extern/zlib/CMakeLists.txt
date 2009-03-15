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
	iowin32.c
	mztools.c
	trees.c
	uncompr.c
	unzip.c
	zip.c
	zutil.c

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