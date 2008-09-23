IF (${OS} STREQUAL "wince")
SET(JPEG_MEM_MGR jmemnobs.c)
ELSE (${OS} STREQUAL "wince")
SET(JPEG_MEM_MGR jmemansi.c)
ENDIF (${OS} STREQUAL "wince")

ADD_LIBRARY(jpeg STATIC
jcapimin.c
jcapistd.c
jccoefct.c
jccolor.c
jcdctmgr.c
jchuff.c
jcinit.c
jcmainct.c
jcmarker.c
jcmaster.c
jcomapi.c
jcparam.c
jcphuff.c
jcprepct.c
jcsample.c
jctrans.c
jdapimin.c
jdapistd.c
jdatadst.c
jdatasrc.c
jdcoefct.c
jdcolor.c
jddctmgr.c
jdhuff.c
jdinput.c
jdmainct.c
jdmarker.c
jdmaster.c
jdmerge.c
jdphuff.c
jdpostct.c
jdsample.c
jdtrans.c
jerror.c
jfdctflt.c
jfdctfst.c
jfdctint.c
jidctflt.c
jidctfst.c
jidctint.c
jidctred.c
jquant1.c
jquant2.c
jutils.c
jmemmgr.c
${JPEG_MEM_MGR}
)

IF (${OS} STREQUAL "wince")
REMOVE_DEFINITIONS(-DHAVE_BOOLEAN)
ADD_DEFINITIONS(-D__inline__= -DNO_GETENV)
ELSE (${OS} STREQUAL "wince")
ADD_DEFINITIONS(-D__inline__=)
ENDIF (${OS} STREQUAL "wince")

INSTALL(TARGETS jpeg ARCHIVE DESTINATION Engine/lib)