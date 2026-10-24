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

TARGET_LINK_LIBRARIES(editor PRIVATE CGE::Engine)

IF (${OS} STREQUAL "windows")
SET_TARGET_PROPERTIES(editor PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

cge_copy_runtime(editor)
INSTALL(TARGETS editor RUNTIME DESTINATION editor LIBRARY DESTINATION editor)
cge_install_runtime(editor editor)
INSTALL(FILES engine.ini DESTINATION editor)
