SET(PRE system)

IF(${OS} STREQUAL "wince")
SET (SYS_FILE cefunctions.cpp)
ENDIF(${OS} STREQUAL "wince")

SET(Engine_system_SRCS
	${PRE}/AtomicCounter.cpp
	${PRE}/bitset.cpp
	#${PRE}/clientsocket.cpp
	${PRE}/engine.cpp
	${PRE}/event.cpp
	${PRE}/file.cpp
	${PRE}/process.cpp
	${PRE}/script.cpp
	#${PRE}/serversocket.cpp
	${PRE}/socket.cpp
	${PRE}/soloader.cpp
	${PRE}/thread.cpp
	${PRE}/utilities.cpp
	${PRE}/${SYS_FILE}
)

SET(Engine_system_HDRS
	${PRE}/AtomicCounter.h
	#${PRE}/clientsocket.h
	${PRE}/bitset.h 
	${PRE}/engine.h
	${PRE}/event.h
	${PRE}/file.h
	${PRE}/process.h
	${PRE}/script.h
	#${PRE}/serversocket.h
	${PRE}/socket.h
	${PRE}/soloader.h
	${PRE}/thread.h
	${PRE}/types.h 
	${PRE}/utilities.h
	${PRE}/allocation.h
)

INSTALL(FILES ${Engine_system_HDRS} DESTINATION Engine/include/${PRE})