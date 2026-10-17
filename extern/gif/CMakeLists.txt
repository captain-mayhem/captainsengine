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
target_include_directories(gif BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(gif PRIVATE HAVE_CONFIG_H)
if (WIN32)
  target_compile_definitions(gif PRIVATE WINDOWS32 HAVE_IO_H)
endif()
cge_extern_defaults(gif)

INSTALL(TARGETS gif ARCHIVE DESTINATION Engine/lib)
