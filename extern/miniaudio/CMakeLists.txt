# miniaudio: single-file audio playback library
add_library(miniaudio STATIC
	miniaudio.c
	miniaudio.h
)
target_include_directories(miniaudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# what the engine's audio module doesn't use: encoding, the waveform
# generators, and the resource manager with its job thread (the engine reads
# the files itself); public, since they change the header's structures
target_compile_definitions(miniaudio PUBLIC MA_NO_ENCODING MA_NO_GENERATION MA_NO_RESOURCE_MANAGER)
if (UNIX AND NOT ANDROID AND NOT EMSCRIPTEN)
	# the Linux backends (ALSA, PulseAudio, JACK) are loaded at runtime
	find_package(Threads REQUIRED)
	target_link_libraries(miniaudio PRIVATE ${CMAKE_DL_LIBS} Threads::Threads m)
elseif (ANDROID)
	# AAudio and OpenSL ES are loaded at runtime
	target_link_libraries(miniaudio PRIVATE ${CMAKE_DL_LIBS})
endif()
if (EMSCRIPTEN)
	# miniaudio 0.11.22 checks the version with the lower-case macros, which
	# emscripten 4 deprecates
	target_compile_options(miniaudio PRIVATE -Wno-deprecated-pragma)
endif()
