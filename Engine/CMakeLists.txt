
INCLUDE(gui/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(io/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(physics/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_LIBRARY(Engine STATIC 
	${Engine_gui_SRCS}
	${Engine_input_SRCS}
	${Engine_io_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_physics_SRCS}
	${Engine_renderer_SRCS}
	${Engine_system_SRCS}
	${Engine_window_SRCS}
	${Engine_common_SRCS}
	
	${Engine_gui_HDRS}
	${Engine_input_HDRS}
	${Engine_io_HDRS}
	${Engine_math_HDRS}
	${Engine_mesh_HDRS}
	${Engine_physics_HDRS}
	${Engine_renderer_HDRS}
	${Engine_system_HDRS}
	${Engine_window_HDRS}
	${Engine_common_HDRS}
)

SOURCE_GROUP("gui" FILES ${Engine_gui_SRCS})
SOURCE_GROUP("gui" FILES ${Engine_gui_HDRS})
SOURCE_GROUP("input" FILES ${Engine_input_SRCS})
SOURCE_GROUP("input" FILES ${Engine_input_HDRS})
SOURCE_GROUP("io" FILES ${Engine_io_SRCS})
SOURCE_GROUP("io" FILES ${Engine_io_HDRS})
SOURCE_GROUP("math" FILES ${Engine_math_SRCS})
SOURCE_GROUP("math" FILES ${Engine_math_HDRS})
SOURCE_GROUP("mesh" FILES ${Engine_mesh_SRCS})
SOURCE_GROUP("mesh" FILES ${Engine_mesh_HDRS})
SOURCE_GROUP("physics" FILES ${Engine_physics_SRCS})
SOURCE_GROUP("physics" FILES ${Engine_physics_HDRS})
SOURCE_GROUP("renderer" FILES ${Engine_renderer_SRCS})
SOURCE_GROUP("renderer" FILES ${Engine_renderer_HDRS})
SOURCE_GROUP("system" FILES ${Engine_system_SRCS})
SOURCE_GROUP("system" FILES ${Engine_system_HDRS})
SOURCE_GROUP("window" FILES ${Engine_window_SRCS})
SOURCE_GROUP("window" FILES ${Engine_window_HDRS})
SOURCE_GROUP("common" FILES ${Engine_common_SRCS})
SOURCE_GROUP("common" FILES ${Engine_common_HDRS})

TARGET_LINK_LIBRARIES(Engine ${ENGINE_LINK_LIBRARIES})

ADD_LIBRARY(EngineMain STATIC
	system/engineMain.cpp
	system/winceMain.h
)
TARGET_LINK_LIBRARIES(EngineMain Engine)
	  
INSTALL(TARGETS Engine EngineMain ARCHIVE DESTINATION Engine/lib)
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)
