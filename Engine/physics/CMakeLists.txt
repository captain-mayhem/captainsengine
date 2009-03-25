SET(PRE physics)

SET(Engine_physics_SRCS
	${PRE}/Body.cpp
	${PRE}/Simulator.cpp
#	${PRE}/
#	${PRE}/
)

SET(Engine_physics_HDRS
	${PRE}/Body.h
	${PRE}/Simulator.h
#	${PRE}/
#	${PRE}/
#	${PRE}/
)

INSTALL(FILES ${Engine_physics_HDRS} DESTINATION Engine/include/${PRE})