SET(PRE mesh)

SET(Engine_mesh_SRCS
	${PRE}/mesh.cpp
	${PRE}/model.cpp
	${PRE}/SimpleMesh.cpp
	${PRE}/triangle.cpp)

SET(Engine_mesh_HDRS
	${PRE}/mesh.h
	${PRE}/model.h
	${PRE}/SimpleMesh.h
	${PRE}/triangle.h)

INSTALL(FILES ${Engine_mesh_HDRS} DESTINATION Engine/include/${PRE})