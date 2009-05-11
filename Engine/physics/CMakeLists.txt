SET(PRE physics)

SET(Engine_physics_SRCS
	${PRE}/Body.cpp
	${PRE}/CollisionBody.cpp
	${PRE}/CollisionHeightfield.cpp
	${PRE}/CollisionObject.cpp
	${PRE}/CollisionPlane.cpp
	${PRE}/CollisionSpace.cpp
	${PRE}/Hinge2Joint.cpp
	${PRE}/Joint.cpp
	${PRE}/Simulator.cpp
)

SET(Engine_physics_HDRS
	${PRE}/Body.h
	${PRE}/CollisionBase.h
	${PRE}/CollisionBody.h
	${PRE}/CollisionHeightfield.h
	${PRE}/CollisionObject.h
	${PRE}/CollisionPlane.h
	${PRE}/CollisionSpace.h
	${PRE}/Hinge2Joint.h
	${PRE}/Joint.h
	${PRE}/Simulator.h
)

INSTALL(FILES ${Engine_physics_HDRS} DESTINATION Engine/include/${PRE})