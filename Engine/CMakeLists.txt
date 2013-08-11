OPTION(RENDER_OPENGL "compile renderer with OpenGL support" ON)
OPTION(RENDER_DIRECTX "compile with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)
OPTION(RENDER_GLES "compile renderer with OpenGL ES support" OFF)
OPTION(RENDER_GL2 "compile renderer with OpenGL (ES) 2.0 support" ON)
OPTION(RENDER_EMBEDDED "use variant of renderer for embedded systems" OFF)
OPTION (NO_PTHREAD_CANCEL "Platform has no pthread cancel" OFF)
IF (OS STREQUAL linux)
OPTION (NO_X11 "Disable X11 for embedded systems" OFF)
ENDIF (OS STREQUAL linux)

IF(RENDER_DIRECTX)
	ADD_DEFINITIONS(-DDIRECTX)
	SET(ENGINE_LINK_LIBRARIES
		d3d9
		d3dx9
		${ENGINE_LINK_LIBRARIES}
	)
ENDIF(RENDER_DIRECTX)
IF(RENDER_OPENGL)
	MESSAGE(STATUS "with OpenGL renderer")
	ADD_DEFINITIONS(-DOPENGL)
ENDIF(RENDER_OPENGL)
IF(RENDER_GL2)
	MESSAGE(STATUS "with OpenGL 2 renderer")
	ADD_DEFINITIONS(-DOPENGL2)
	SET(ENGINE_COMPILE_FLAGS
		-DGLEW_STATIC
	)
	SET(ENGINE_LINK_LIBRARIES
		${ENGINE_LINK_LIBRARIES}
		glew
	)
ENDIF(RENDER_GL2)
IF (RENDER_OPENGL OR RENDER_GL2)
	INCLUDE(FindOpenGL)
	SET(ENGINE_LINK_LIBRARIES
		${OPENGL_LIBRARIES}
		${ENGINE_LINK_LIBRARIES}
	)
ENDIF(RENDER_OPENGL OR RENDER_GL2)
IF (RENDER_EMBEDDED)
	MESSAGE(STATUS "with OpenGL ES 2.0 renderer")
	ADD_DEFINITIONS(-DOPENGL2)
ENDIF (RENDER_EMBEDDED)
IF(RENDER_GLES)
	ADD_DEFINITIONS(-DGLES)
IF (${OS} STREQUAL "qnx")
	SET(ENGINE_LINK_LIBRARIES
		EGL
		GLESv1_CM
		gf
		${ENGINE_LINK_LIBRARIES}
	)
ELSE (${OS} STREQUAL "qnx")
	SET(ENGINE_LINK_LIBRARIES
		libEGL
		libGLES_CM_NoE
		${ENGINE_LINK_LIBRARIES}
	)
ENDIF (${OS} STREQUAL "qnx")
ENDIF(RENDER_GLES)

IF (OS STREQUAL linux)
IF (NOT NO_X11)
SET(ENGINE_LINK_LIBRARIES
  ${ENGINE_LINK_LIBRARIES}
  X11
  Xxf86vm
)
ELSE (NOT NO_X11)
ADD_DEFINITIONS(
-DNO_X11
)
ENDIF (NOT NO_X11)
ENDIF (OS STREQUAL linux)

INCLUDE(gui/CMakeLists.txt)
INCLUDE(image/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(io/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(physics/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)
INCLUDE(util/CMakeLists.txt)

INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

IF (NO_PTHREAD_CANCEL)
ADD_DEFINITIONS(-DNO_PTHREAD_CANCEL)
ENDIF (NO_PTHREAD_CANCEL)

ADD_DEFINITIONS(${ENGINE_COMPILE_FLAGS})

ADD_LIBRARY(Engine STATIC 
	${Engine_gui_SRCS}
	${Engine_image_SRCS}
	${Engine_input_SRCS}
	${Engine_io_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_physics_SRCS}
	${Engine_renderer_SRCS}
	${Engine_system_SRCS}
	${Engine_window_SRCS}
	${Engine_common_SRCS}
	${Engine_util_SRCS}
	
	${Engine_gui_HDRS}
	${Engine_image_HDRS}
	${Engine_input_HDRS}
	${Engine_io_HDRS}
	${Engine_math_HDRS}
	${Engine_mesh_HDRS}
	${Engine_physics_HDRS}
	${Engine_renderer_HDRS}
	${Engine_system_HDRS}
	${Engine_window_HDRS}
	${Engine_common_HDRS}
	${Engine_util_HDRS}
)

TARGET_LINK_LIBRARIES(Engine ${ENGINE_LINK_LIBRARIES})

SOURCE_GROUP("gui" FILES ${Engine_gui_SRCS})
SOURCE_GROUP("gui" FILES ${Engine_gui_HDRS})
SOURCE_GROUP("image" FILES ${Engine_image_SRCS})
SOURCE_GROUP("image" FILES ${Engine_image_HDRS})
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
SOURCE_GROUP("util" FILES ${Engine_util_SRCS})
SOURCE_GROUP("util" FILES ${Engine_util_HDRS})

TARGET_LINK_LIBRARIES(Engine ${ENGINE_LINK_LIBRARIES})

ADD_LIBRARY(EngineMain STATIC
	system/engineMain.cpp
	system/winceMain.h
)
TARGET_LINK_LIBRARIES(EngineMain Engine)

SET(ENGINE_LINK_LIBRARIES ${ENGINE_LINK_LIBRARIES} PARENT_SCOPE)
SET(ENGINE_COMPILE_FLAGS ${ENGINE_COMPILE_FLAGS} PARENT_SCOPE)
	  
INSTALL(TARGETS Engine EngineMain ARCHIVE DESTINATION Engine/lib)
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)
