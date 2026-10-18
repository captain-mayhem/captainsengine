# miniaudio: single-file audio playback library
add_library(miniaudio STATIC
	miniaudio.c
	miniaudio.h
)
target_include_directories(miniaudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (UNIX AND NOT ANDROID AND NOT EMSCRIPTEN)
	# the Linux backends (ALSA, PulseAudio, JACK) are loaded at runtime
	find_package(Threads REQUIRED)
	target_link_libraries(miniaudio PRIVATE ${CMAKE_DL_LIBS} Threads::Threads m)
endif()
