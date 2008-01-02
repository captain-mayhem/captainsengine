INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

SET(editor_SRCS
	editor.cpp
	graphics.cpp
	hqmExport.cpp
	main.cpp
	menu.cpp)

ADD_EXECUTABLE(editor ${editor_SRCS})

TARGET_LINK_LIBRARIES(editor Engine)
SET_TARGET_PROPERTIES(editor PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")

INSTALL(TARGETS editor RUNTIME DESTINATION editor)
INSTALL(FILES engine.ini DESTINATION editor)
INSTALL(FILES textures/font/font.bmp DESTINATION editor/textures/font)