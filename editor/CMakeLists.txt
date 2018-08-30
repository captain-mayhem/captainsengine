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

if (OS STREQUAL "android")
add_library(editor SHARED
${editor_SRCS} ${editor_HDRS})
else()
ADD_EXECUTABLE(editor ${editor_SRCS} ${editor_HDRS})
endif()

TARGET_LINK_LIBRARIES(editor EngineMain ${ENGINE_LINK_LIBRARIES})

IF (${OS} STREQUAL "windows")
SET_TARGET_PROPERTIES(editor PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS editor RUNTIME DESTINATION editor LIBRARY DESTINATION editor)
INSTALL(FILES engine.ini DESTINATION editor)
INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)