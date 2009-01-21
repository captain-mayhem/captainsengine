SET(PRE io)

SET(Engine_io_SRCS
	${PRE}/eventqueue.cpp
)

SET(Engine_io_HDRS
	${PRE}/eventqueue.h
)

INSTALL(FILES ${Engine_io_HDRS} DESTINATION Engine/include/${PRE})