SET(PRE math)

SET(Engine_math_SRCS
	${PRE}/arcball.cpp
	${PRE}/bbox.cpp
	${PRE}/bsphere.cpp
	${PRE}/frustum.cpp
	${PRE}/matrix.cpp
	${PRE}/plane.cpp
#	${PRE}/quaternion.cpp
	${PRE}/ray.cpp
	${PRE}/vector.cpp)

SET(Engine_math_HDRS
	${PRE}/arcball.h 
	${PRE}/bbox.h 
	${PRE}/boundingobject.h 
	${PRE}/bsphere.h 
	${PRE}/frustum.h
	${PRE}/matrix.h
	${PRE}/plane.h
	${PRE}/quaternion.h 
	${PRE}/ray.h 
	${PRE}/vector.h)

INSTALL(FILES ${Engine_math_HDRS} DESTINATION Engine/include/${PRE})