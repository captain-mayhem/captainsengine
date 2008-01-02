SET(PRE system)

SET(Engine_system_SRCS
	${PRE}/bitset.cpp
	${PRE}/engine.cpp
	${PRE}/file.cpp
	${PRE}/gameobject.cpp
	${PRE}/script.cpp
	${PRE}/thread.cpp
	${PRE}/utilities.cpp)

INSTALL(FILES 	
	${PRE}/bitset.h 
	${PRE}/engine.h 
	${PRE}/file.h 
	${PRE}/gameobject.h 
	${PRE}/script.h 
	${PRE}/thread.h 
	${PRE}/utilities.h 
DESTINATION Engine/include/${PRE})