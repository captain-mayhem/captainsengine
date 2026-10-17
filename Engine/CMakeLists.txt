# Captain's Game Engine library

if (OS STREQUAL android)
  option(RENDER_OPENGL "compile renderer with OpenGL support" OFF)
  option(RENDER_DIRECTX "compile with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)
  option(RENDER_GL2 "compile renderer with OpenGL (ES) 2.0 support" ON)
  option(RENDER_EMBEDDED "use variant of OpenGL ES 2.0 renderer for embedded systems" ON)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" ON)
else()
  option(RENDER_OPENGL "compile renderer with OpenGL support" ON)
  option(RENDER_DIRECTX "compile with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)
  option(RENDER_GL2 "compile renderer with OpenGL (ES) 2.0 support" ON)
  option(RENDER_EMBEDDED "use variant of OpenGL ES 2.0 renderer for embedded systems" OFF)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" OFF)
endif()
if (OS STREQUAL linux)
  option(DISABLE_XF86VMODE "Disables the X11 xf86 vmode extension" OFF)
endif()

INCLUDE(gui/CMakeLists.txt)
INCLUDE(image/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(io/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(physics/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(renderpass/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)
INCLUDE(util/CMakeLists.txt)

ADD_LIBRARY(Engine STATIC 
	${Engine_gui_SRCS}
	${Engine_image_SRCS}
	${Engine_input_SRCS}
	${Engine_io_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_physics_SRCS}
	${Engine_renderer_SRCS}
	${Engine_renderpass_SRCS}
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
	${Engine_renderpass_HDRS}
	${Engine_system_HDRS}
	${Engine_window_HDRS}
	${Engine_common_HDRS}
	${Engine_util_HDRS}
)
add_library(CGE::Engine ALIAS Engine)
target_compile_features(Engine PUBLIC cxx_std_20)

# Headers are included as <system/engine.h> (relative to Engine/) and a few
# as <extern/...> (relative to the engine root).
target_include_directories(Engine PUBLIC
	${CaptainsEngine_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

# platform definitions used in the public headers
if (OS STREQUAL "windows")
	target_compile_definitions(Engine PUBLIC _CRT_SECURE_NO_DEPRECATE)
	target_link_libraries(Engine PUBLIC wsock32 winmm)
elseif (OS STREQUAL "linux")
	target_compile_definitions(Engine PUBLIC UNIX LINUX __STDC_LIMIT_MACROS)
	find_package(Threads REQUIRED)
	target_link_libraries(Engine PUBLIC Threads::Threads rt)
elseif (OS STREQUAL "android")
	target_compile_definitions(Engine PUBLIC UNIX ANDROID)
	target_link_libraries(Engine PUBLIC android log)
endif()

# Lua (system/script.h), ODE (physics/*.h) and zlib (io/ZipReader.h) appear in
# the public headers; the image libraries are only used inside the engine.
target_link_libraries(Engine
	PUBLIC lua lualib ode zlib
	PRIVATE jpeg gif png
)

# renderers; these definitions are internal to the engine
if (RENDER_DIRECTX)
	target_compile_definitions(Engine PRIVATE DIRECTX)
	target_link_libraries(Engine PRIVATE d3d11 d3dcompiler dxguid)
endif()
if (RENDER_OPENGL)
	message(STATUS "with OpenGL renderer")
	target_compile_definitions(Engine PRIVATE OPENGL)
endif()
if (RENDER_GL2)
	message(STATUS "with OpenGL 2 renderer")
	target_compile_definitions(Engine PRIVATE OPENGL2)
	if (NOT OS STREQUAL "android")
		# the GL2 renderer headers include <GL/glew.h>
		target_link_libraries(Engine PUBLIC glew)
	endif()
endif()
if ((RENDER_OPENGL OR RENDER_GL2) AND NOT RENDER_EMBEDDED)
	find_package(OpenGL REQUIRED)
	target_link_libraries(Engine PUBLIC OpenGL::GL OpenGL::GLU)
	if (TARGET OpenGL::GLX)
		target_link_libraries(Engine PUBLIC OpenGL::GLX)
	endif()
endif()
if (RENDER_EMBEDDED)
	message(STATUS "with OpenGL ES 2.0 renderer")
	target_compile_definitions(Engine PRIVATE OPENGL2 RENDER_ES20)
	target_compile_definitions(Engine PUBLIC RENDER_EMBEDDED)
	target_link_libraries(Engine PUBLIC GLESv2 EGL)
endif()

if (OS STREQUAL linux)
	target_link_libraries(Engine PUBLIC X11)
	if (DISABLE_XF86VMODE)
		target_compile_definitions(Engine PUBLIC DISABLE_XF86VMODE)
	else()
		target_link_libraries(Engine PUBLIC Xxf86vm)
	endif()
endif()

if (NO_PTHREAD_CANCEL)
	target_compile_definitions(Engine PRIVATE NO_PTHREAD_CANCEL)
endif()

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
SOURCE_GROUP("renderpass" FILES ${Engine_renderpass_SRCS})
SOURCE_GROUP("renderpass" FILES ${Engine_renderpass_HDRS})
SOURCE_GROUP("system" FILES ${Engine_system_SRCS})
SOURCE_GROUP("system" FILES ${Engine_system_HDRS})
SOURCE_GROUP("window" FILES ${Engine_window_SRCS})
SOURCE_GROUP("window" FILES ${Engine_window_HDRS})
SOURCE_GROUP("common" FILES ${Engine_common_SRCS})
SOURCE_GROUP("common" FILES ${Engine_common_HDRS})
SOURCE_GROUP("util" FILES ${Engine_util_SRCS})
SOURCE_GROUP("util" FILES ${Engine_util_HDRS})


if (OS STREQUAL "android")
find_file(ENGINE_MAIN_ADD_SRC android_native_app_glue.c)
endif()

ADD_LIBRARY(EngineMain STATIC
	system/engineMain.cpp
	${ENGINE_MAIN_ADD_SRC}
)
TARGET_LINK_LIBRARIES(EngineMain PUBLIC Engine)
if (OS STREQUAL "android")
	# keep android_main from the native app glue in the application library
	target_link_options(EngineMain INTERFACE -Wl,--no-undefined -u android_main)
endif()
	  
INSTALL(TARGETS Engine EngineMain ARCHIVE DESTINATION Engine/lib)
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)