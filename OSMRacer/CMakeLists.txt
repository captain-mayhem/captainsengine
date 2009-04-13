INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/libxml)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(OSMRacer_SRCS
	main.cpp
	GeoCache.cpp
	GeoGen.cpp
	OSMReader.cpp
	MapChunk.cpp
	StreetGenerator.cpp
	Terrain.cpp
	TerrainChunk.cpp
	Utilities.cpp
	Vehicle.cpp
	)

SET(OSMRacer_HDRS
	GeoCache.h
	GeoGen.h
	OSMReader.h
	MapChunk.h
	Renderable.h
	StreetGenerator.h
	Terrain.h
	TerrainChunk.h
	Utilities.h
	Vehicle.h
	)

ADD_EXECUTABLE(OSMRacer ${OSMRacer_SRCS} ${OSMRacer_HDRS})

TARGET_LINK_LIBRARIES(OSMRacer EngineMain xml)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(OSMRacer PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

#INSTALL(TARGETS editor RUNTIME DESTINATION editor)
#INSTALL(FILES engine.ini DESTINATION editor)
#INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)
