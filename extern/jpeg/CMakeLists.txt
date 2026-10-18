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
jmemansi.c
)
target_include_directories(jpeg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
	# jconfig.h uses the GCC keyword __inline__ for INLINE; elsewhere redefining it
	# would also break the C library's inline functions (bionic's fortified strlen)
	target_compile_definitions(jpeg PRIVATE __inline__=)
endif()
cge_extern_defaults(jpeg)

INSTALL(TARGETS jpeg ARCHIVE DESTINATION Engine/lib)
