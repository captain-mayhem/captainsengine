SET(PRE input)

SET(Engine_input_SRCS
	${PRE}/keyboard.cpp
	${PRE}/mouse.cpp)

SET(Engine_input_HDRS
	${PRE}/keyboard.h
	${PRE}/mouse.h
)

INSTALL(FILES ${Engine_input_HDRS} DESTINATION Engine/include/${PRE})