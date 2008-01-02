SET(PRE input)

SET(Engine_input_SRCS
	${PRE}/keyboard.cpp
	${PRE}/mouse.cpp)

INSTALL(FILES 	
	${PRE}/keyboard.h
	${PRE}/mouse.h
DESTINATION Engine/include/${PRE})