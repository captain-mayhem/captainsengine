SET(PRE system)

IF(${OS} STREQUAL "wince")
SET (SYS_FILE cefunctions.cpp)
ENDIF(${OS} STREQUAL "wince")

SET(Engine_system_SRCS
	${PRE}/bitset.cpp
	${PRE}/engine.cpp
	${PRE}/file.cpp
	${PRE}/gameobject.cpp
	${PRE}/script.cpp
	${PRE}/thread.cpp
	${PRE}/utilities.cpp
	${PRE}/${SYS_FILE}
)

SET(Engine_system_HDRS
	${PRE}/bitset.h 
	${PRE}/engine.h 
	${PRE}/file.h 
	${PRE}/gameobject.h 
	${PRE}/script.h 
	${PRE}/thread.h 
	${PRE}/utilities.h)

INSTALL(FILES ${Engine_system_HDRS} DESTINATION Engine/include/${PRE})