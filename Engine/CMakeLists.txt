INCLUDE(FindOpenGL)

INCLUDE(gui/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

IF(NOT ${DIRECTX} AND ${OS} STREQUAL "windows")
	ADD_DEFINITIONS(/D_NODIRECTX)	
ENDIF(NOT ${DIRECTX} AND ${OS} STREQUAL "windows")

ADD_LIBRARY(Engine STATIC 
	${Engine_gui_SRCS}
	${Engine_input_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_renderer_SRCS}
	${Engine_system_SRCS}
	${Engine_window_SRCS}
	${Engine_common_SRCS}
	
	${Engine_gui_HDRS}
	${Engine_input_HDRS}
	${Engine_math_HDRS}
	${Engine_mesh_HDRS}
	${Engine_renderer_HDRS}
	${Engine_system_HDRS}
	${Engine_window_HDRS}
	${Engine_common_HDRS}
)

SOURCE_GROUP("gui" FILES ${Engine_gui_SRCS})
SOURCE_GROUP("gui" FILES ${Engine_gui_HDRS})
SOURCE_GROUP("input" FILES ${Engine_input_SRCS})
SOURCE_GROUP("input" FILES ${Engine_input_HDRS})
SOURCE_GROUP("math" FILES ${Engine_math_SRCS})
SOURCE_GROUP("math" FILES ${Engine_math_HDRS})
SOURCE_GROUP("mesh" FILES ${Engine_mesh_SRCS})
SOURCE_GROUP("mesh" FILES ${Engine_mesh_HDRS})
SOURCE_GROUP("renderer" FILES ${Engine_renderer_SRCS})
SOURCE_GROUP("renderer" FILES ${Engine_renderer_HDRS})
SOURCE_GROUP("system" FILES ${Engine_system_SRCS})
SOURCE_GROUP("system" FILES ${Engine_system_HDRS})
SOURCE_GROUP("window" FILES ${Engine_window_SRCS})
SOURCE_GROUP("window" FILES ${Engine_window_HDRS})
SOURCE_GROUP("common" FILES ${Engine_common_SRCS})
SOURCE_GROUP("common" FILES ${Engine_common_HDRS})

IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(Engine
	  jpeg
	  liblua
	  liblualib
	  d3d9
	  d3dx9
	  ${OPENGL_LIBRARIES})
ENDIF (${OS} STREQUAL "windows")
IF (${OS} STREQUAL "linux")
  TARGET_LINK_LIBRARIES(Engine
	  jpeg
	  lua
	  lualib
	  ${OPENGL_LIBRARIES}
    X11
    Xxf86vm
    pthread)
ENDIF (${OS} STREQUAL "linux")


INSTALL(TARGETS Engine ARCHIVE DESTINATION Engine/lib)
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)
