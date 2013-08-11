SET(PRE util)

SET(Engine_util_SRCS
	${PRE}/activeobject.cpp
)

SET(Engine_util_HDRS
	${PRE}/activeobject.h
)

INSTALL(FILES ${Engine_util_HDRS} DESTINATION Engine/include/${PRE})