INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/libxml)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(OSMRacer_SRCS
	main.cpp
	OSMReader.cpp
	MapChunk.cpp
	Utilities.cpp
	)

SET(OSMRacer_HDRS
	OSMReader.h
	MapChunk.h
	Utilities.h
	)

ADD_EXECUTABLE(OSMRacer ${OSMRacer_SRCS} ${OSMRacer_HDRS})

TARGET_LINK_LIBRARIES(OSMRacer Engine xml)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(OSMRacer PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

#INSTALL(TARGETS editor RUNTIME DESTINATION editor)
#INSTALL(FILES engine.ini DESTINATION editor)
#INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)
