SET(PRE common)

SET(Engine_common_SRCS
	${PRE}/graph.cpp)

SET(Engine_common_HDRS
	${PRE}/graph.h
	${PRE}/octree.h
	${PRE}/OctreeStatic.h
	${PRE}/RefCountedObject.h
)

INSTALL(FILES ${Engine_common_HDRS} DESTINATION Engine/include/${PRE})