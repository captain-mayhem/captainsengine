INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(HQEditor_SRCS
	compiler.cpp
	../HQClient/door.cpp
	main.cpp
	../../editor/hqmExport.cpp
	../../editor/hqmExport.h
)

SET(HQEditor_HDRS
	compiler.h)

ADD_EXECUTABLE("HQEditor" ${HQEditor_SRCS} ${HQEditor_HDRS})

TARGET_LINK_LIBRARIES(HQEditor EngineMain ${ENGINE_LINK_LIBRARIES})
IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(HQEditor PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS HQEditor RUNTIME DESTINATION HQEditor)
INSTALL(FILES engine.ini SCRIPT.txt DESTINATION HQEditor)
INSTALL(FILES make.bat copy levels DESTINATION HQEditor PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
INSTALL(FILES textures/font/font.bmp DESTINATION HQEditor/textures/font)
INSTALL(DIRECTORY basic DESTINATION HQEditor PATTERN ".svn" EXCLUDE)
