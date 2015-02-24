INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(editor_SRCS
	camera.cpp
	editor.cpp
	graphics.cpp
	hqmExport.cpp
	main.cpp
	menu.cpp)

SET(editor_HDRS
	camera.h
	editor.h
	graphics.h
	hqmExport.h
	menu.h)

ADD_EXECUTABLE(editor ${editor_SRCS} ${editor_HDRS})

TARGET_LINK_LIBRARIES(editor EngineMain ${ENGINE_LINK_LIBRARIES})

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(editor PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS editor RUNTIME DESTINATION editor)
INSTALL(FILES engine.ini DESTINATION editor)
INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)
