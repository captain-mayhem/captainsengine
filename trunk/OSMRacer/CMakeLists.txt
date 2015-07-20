INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/libxml)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(OSMRacer_SRCS
	main.cpp
	FollowCam.cpp
	OSMMapAdapter.cpp
	OSMReader.cpp
	MapChunk.cpp
	SimpleMapAdapter.cpp
	StreetGenerator.cpp
	Terrain.cpp
	TerrainChunk.cpp
	Utilities.cpp
	Vehicle.cpp
	)

SET(OSMRacer_HDRS
	FollowCam.h
	GameObject.h
	OSMMapAdapter.h
	OSMReader.h
	MapChunk.h
	MapInfo.h
	Renderable.h
	SimpleMapAdapter.h
	StreetGenerator.h
	Terrain.h
	TerrainChunk.h
	Utilities.h
	Vehicle.h
	)

ADD_EXECUTABLE(OSMRacer ${OSMRacer_SRCS} ${OSMRacer_HDRS})

TARGET_LINK_LIBRARIES(OSMRacer EngineMain ${ENGINE_LINK_LIBRARIES} xml)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(OSMRacer PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

#INSTALL(TARGETS editor RUNTIME DESTINATION editor)
#INSTALL(FILES engine.ini DESTINATION editor)
#INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)
