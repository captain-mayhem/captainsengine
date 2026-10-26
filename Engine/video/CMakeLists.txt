SET(PRE video)

SET(Engine_video_SRCS
	${PRE}/mpegdecoder.cpp
	${PRE}/theoradecoder.cpp
	${PRE}/video.cpp
	${PRE}/videodecoder.cpp
)

SET(Engine_video_HDRS
	${PRE}/video.h
	${PRE}/videodecoder.h
)

# internal to the engine (the decoders' side), not installed
SET(Engine_video_internal_HDRS
	${PRE}/video_internal.h
)

INSTALL(FILES ${Engine_video_HDRS} DESTINATION Engine/include/${PRE})
