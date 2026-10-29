SET(PRE audio)

SET(Engine_audio_SRCS
	${PRE}/audio.cpp
	${PRE}/audiodecoder.cpp
	${PRE}/audioeffect.cpp
	${PRE}/audioplayer.cpp
	${PRE}/audiostream.cpp
	${PRE}/decoder.cpp
)

SET(Engine_audio_HDRS
	${PRE}/audio.h
	${PRE}/audiodecoder.h
	${PRE}/audioeffect.h
	${PRE}/audioplayer.h
	${PRE}/audiostream.h
)

# internal to the engine (miniaudio's side), not installed
SET(Engine_audio_internal_HDRS
	${PRE}/audio_internal.h
)

INSTALL(FILES ${Engine_audio_HDRS} DESTINATION Engine/include/${PRE})
