INCLUDE(FindOpenGL)

INCLUDE(gui/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

IF(NOT ${DIRECTX})
	ADD_DEFINITIONS(/D_NODIRECTX)	
ENDIF(NOT ${DIRECTX})

ADD_LIBRARY(Engine STATIC 
	${Engine_gui_SRCS}
	${Engine_input_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_renderer_SRCS}
	${Engine_system_SRCS}
	${Engine_window_SRCS}
	
	${Engine_math_HDRS}
	${Engine_mesh_HDRS}
	)

SOURCE_GROUP("math\\Source Files" FILES ${Engine_math_SRCS})
SOURCE_GROUP("math\\Header Files" FILES ${Engine_math_HDRS})
SOURCE_GROUP("mesh\\Source Files" FILES ${Engine_mesh_SRCS})
SOURCE_GROUP("mesh\\Header Files" FILES ${Engine_mesh_HDRS})

TARGET_LINK_LIBRARIES(Engine
	jpeg
	liblua
	liblualib
	d3d9
	d3dx9
	${OPENGL_LIBRARIES})

INSTALL(TARGETS Engine ARCHIVE DESTINATION Engine/lib)
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)