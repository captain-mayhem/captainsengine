SET(PRE util)

SET(Engine_util_SRCS
	${PRE}/activeobject.cpp
)

SET(Engine_util_HDRS
	${PRE}/activeobject.h
)

cge_install_sdk(FILES ${Engine_util_HDRS} DESTINATION Engine/include/${PRE})