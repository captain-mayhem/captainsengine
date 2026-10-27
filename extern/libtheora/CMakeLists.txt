# libtheora 1.2.0 (Xiph.org, BSD; https://www.theora.org), the decoder and the
# encoder, without the x86/ARM/C64x assembly (plain C on every platform), and
# only lib/, include/ and the license files of the release
set(THEORA_COMMON_SOURCES
	lib/apiwrapper.c
	lib/bitpack.c
	lib/dequant.c
	lib/fragment.c
	lib/idct.c
	lib/info.c
	lib/internal.c
	lib/quant.c
	lib/state.c
)

add_library(theoradec STATIC
	${THEORA_COMMON_SOURCES}
	lib/decapiwrapper.c
	lib/decinfo.c
	lib/decode.c
	lib/huffdec.c
)

add_library(theoraenc STATIC
	${THEORA_COMMON_SOURCES}
	lib/analyze.c
	lib/encapiwrapper.c
	lib/encfrag.c
	lib/encinfo.c
	lib/encode.c
	lib/enquant.c
	lib/fdct.c
	lib/huffenc.c
	lib/mathops.c
	lib/mcenc.c
	lib/rate.c
	lib/tokenize.c
)

foreach(target theoradec theoraenc)
	target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_link_libraries(${target} PUBLIC ogg)
	cge_extern_defaults(${target})
	if (NOT MSVC)
		target_link_libraries(${target} PRIVATE m)
	endif()
endforeach()
