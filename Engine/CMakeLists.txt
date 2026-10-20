# Captain's Game Engine library

if (OS STREQUAL android)
  option(RENDER_OPENGL "compile renderer with OpenGL support" OFF)
  option(RENDER_GL2 "compile renderer with OpenGL (ES) 2.0 support" ON)
  option(RENDER_EMBEDDED "use variant of OpenGL ES 2.0 renderer for embedded systems" ON)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" ON)
else()
  option(RENDER_OPENGL "compile renderer with OpenGL support" ON)
  option(RENDER_GL2 "compile renderer with OpenGL (ES) 2.0 support" ON)
  option(RENDER_EMBEDDED "use variant of OpenGL ES 2.0 renderer for embedded systems" OFF)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" OFF)
endif()
if (OS STREQUAL linux)
  option(DISABLE_XF86VMODE "Disables the X11 xf86 vmode extension" OFF)
endif()
# Without threads the engine and its applications do their work in the main
# loop (the browser's main thread on the web).
if (EMSCRIPTEN)
  set(ENGINE_SINGLE_THREADED_DEFAULT ON)
else()
  set(ENGINE_SINGLE_THREADED_DEFAULT OFF)
endif()
option(ENGINE_SINGLE_THREADED "build the engine and its applications without threads" ${ENGINE_SINGLE_THREADED_DEFAULT})

INCLUDE(gui/CMakeLists.txt)
INCLUDE(image/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(io/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(platform/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(renderpass/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)
INCLUDE(util/CMakeLists.txt)

# The engine is a shared library (CGE.dll / libCGE.so), so that applications
# link the LGPL code dynamically. Only what is marked CGE_API is exported.
ADD_LIBRARY(Engine SHARED
	${Engine_gui_SRCS}
	${Engine_image_SRCS}
	${Engine_input_SRCS}
	${Engine_io_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_platform_SRCS}
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
	${Engine_platform_HDRS}
	${Engine_platform_internal_HDRS}
	${Engine_renderer_HDRS}
	${Engine_renderpass_HDRS}
	${Engine_system_HDRS}
	${Engine_window_HDRS}
	${Engine_common_HDRS}
	${Engine_util_HDRS}
)
add_library(CGE::Engine ALIAS Engine)
target_compile_features(Engine PUBLIC cxx_std_20)
set_target_properties(Engine PROPERTIES
	OUTPUT_NAME CGE
	CXX_VISIBILITY_PRESET hidden
	C_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

include(GenerateExportHeader)
generate_export_header(Engine
	BASE_NAME CGE
	EXPORT_MACRO_NAME CGE_API
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/cge_export.h
)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
if (MSVC)
	# exported classes with STL members or bases; fine because everything uses
	# the same compiler and the DLL runtime
	target_compile_options(Engine PUBLIC /wd4251 /wd4275)
endif()
if (OS STREQUAL "linux" OR OS STREQUAL "android")
	# don't re-export the statically linked third-party libraries
	target_link_options(Engine PRIVATE -Wl,--exclude-libs,ALL)
endif()

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
	# user folders (SHGetKnownFolderPath) and ShellExecute
	target_link_libraries(Engine PRIVATE shell32 ole32)
elseif (OS STREQUAL "linux")
	target_compile_definitions(Engine PUBLIC UNIX LINUX __STDC_LIMIT_MACROS)
	find_package(Threads REQUIRED)
	target_link_libraries(Engine PUBLIC Threads::Threads rt)
elseif (OS STREQUAL "android")
	target_compile_definitions(Engine PUBLIC UNIX ANDROID)
	target_link_libraries(Engine PUBLIC android log)
endif()

if (ENGINE_SINGLE_THREADED)
	target_compile_definitions(Engine PUBLIC ENGINE_SINGLE_THREADED)
endif()

# Third-party libraries are linked statically into the engine library.
# Lua is compiled into it and exported (applications use the Lua API
# directly). The others are private; the zlib and minizip headers appear in
# io/GZipReader.h and io/ZipReader.h, so only their include directories are
# passed on.
target_link_libraries(Engine
	PUBLIC lua
	PRIVATE minizip ZLIB::ZLIB JPEG::JPEG PNG::PNG gif
)
if (WIN32)
	# applications see the Lua API as dllimport (see extern/lua)
	target_compile_definitions(Engine INTERFACE LUA_BUILD_AS_DLL)
endif()
function(cge_engine_public_headers lib)
	target_include_directories(Engine PUBLIC $<TARGET_PROPERTY:${lib},INTERFACE_INCLUDE_DIRECTORIES>)
	target_compile_definitions(Engine PUBLIC $<TARGET_PROPERTY:${lib},INTERFACE_COMPILE_DEFINITIONS>)
endfunction()
cge_engine_public_headers(zlibstatic)
cge_engine_public_headers(minizip)

# the render backends: OpenGL (renderer/GL/)
INCLUDE(renderer/GL/CMakeLists.txt)

# the old renderers, which draw on the OpenGL backend's context until they
# are replaced; these definitions are internal to the engine
if (RENDER_OPENGL)
	message(STATUS "with OpenGL renderer")
	target_compile_definitions(Engine PRIVATE OPENGL)
endif()
if (RENDER_GL2)
	message(STATUS "with OpenGL 2 renderer")
	target_compile_definitions(Engine PRIVATE OPENGL2)
endif()
if ((RENDER_OPENGL OR RENDER_GL2) AND NOT RENDER_EMBEDDED)
	# the old renderers load their own copy of glad (renderer/OpenGL/OGLfunctions.cpp);
	# OpenGL::GL stays public for projects that still call fixed-function GL
	find_package(OpenGL REQUIRED)
	target_link_libraries(Engine PRIVATE glad PUBLIC OpenGL::GL OpenGL::GLU)
	if (OS STREQUAL linux)
		# the GL backend's context is an EGL context, so the functions come from EGL
		find_package(OpenGL REQUIRED COMPONENTS EGL)
		target_link_libraries(Engine PRIVATE OpenGL::EGL)
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
SOURCE_GROUP("platform" FILES ${Engine_platform_SRCS})
SOURCE_GROUP("platform" FILES ${Engine_platform_HDRS} ${Engine_platform_internal_HDRS})
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


# Applications define their entry point with CGE_MAIN (system/application.h).
# On Android that is android_main, which the NDK's native app glue calls; the
# glue is compiled into the application's library, not into the engine.
if (OS STREQUAL "android")
	# ANDROID_NDK comes from the NDK's toolchain file
	set(ANDROID_NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
	add_library(cge_android_glue STATIC ${ANDROID_NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
	target_include_directories(cge_android_glue PUBLIC ${ANDROID_NATIVE_APP_GLUE_DIR})
	target_link_libraries(cge_android_glue PUBLIC android log)
	# engine headers include android_native_app_glue.h
	target_include_directories(Engine PUBLIC ${ANDROID_NATIVE_APP_GLUE_DIR})
	# keep the glue's entry point, which nothing in the application references
	target_link_options(cge_android_glue INTERFACE -u ANativeActivity_onCreate)
	target_link_libraries(Engine INTERFACE cge_android_glue)
endif()
	  
# optional physics library on top of the engine; ODE appears in its public headers
if (CGE_WITH_PHYSICS)
	INCLUDE(physics/CMakeLists.txt)
	add_library(EnginePhysics STATIC ${Engine_physics_SRCS} ${Engine_physics_HDRS})
	add_library(CGE::Physics ALIAS EnginePhysics)
	target_link_libraries(EnginePhysics PUBLIC Engine ode)
	SOURCE_GROUP("physics" FILES ${Engine_physics_SRCS} ${Engine_physics_HDRS})
	INSTALL(TARGETS EnginePhysics ARCHIVE DESTINATION Engine/lib)
endif()


# Runtime helpers for applications

# The render backend plugins are loaded at runtime, so applications don't
# link them; these functions build, copy and install them with the engine.

# copies the engine library and the backend plugins next to an application's
# executable after building, so it can be started from the build folder
# (Windows has no RPATH; elsewhere the plugins are next to the engine library)
function(cge_copy_runtime target)
	if (TARGET EngineGL)
		add_dependencies(${target} EngineGL)
	endif()
	if (WIN32)
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE:EngineGL> $<TARGET_FILE_DIR:${target}>
			COMMAND_EXPAND_LISTS
		)
	endif()
endfunction()

# installs the engine library and the backend plugins for an application
# installed to <destination>: next to the executable on Windows, in lib/
# (found through RPATH) elsewhere
function(cge_install_runtime target destination)
	if (WIN32)
		install(TARGETS Engine RUNTIME DESTINATION ${destination} ${ARGN})
		if (TARGET EngineGL)
			install(TARGETS EngineGL LIBRARY DESTINATION ${destination} ${ARGN})
		endif()
	else()
		install(TARGETS Engine LIBRARY DESTINATION ${destination}/lib ${ARGN})
		if (TARGET EngineGL)
			install(TARGETS EngineGL LIBRARY DESTINATION ${destination}/lib ${ARGN})
		endif()
		set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
	endif()
endfunction()
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)