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

if(OS STREQUAL "android")
ADD_LIBRARY(OSMRacer SHARED ${OSMRacer_SRCS} ${OSMRacer_HDRS})
else()
ADD_EXECUTABLE(OSMRacer ${OSMRacer_SRCS} ${OSMRacer_HDRS})
endif()

TARGET_LINK_LIBRARIES(OSMRacer PRIVATE CGE::Engine CGE::Physics xml)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(OSMRacer PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

cge_copy_runtime(OSMRacer)

#INSTALL(TARGETS editor RUNTIME DESTINATION editor LIBRARY DESTINATION editor)
#INSTALL(FILES engine.ini DESTINATION editor)
