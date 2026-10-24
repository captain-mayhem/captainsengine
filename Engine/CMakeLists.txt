# Captain's Game Engine library

if (OS STREQUAL android)
  option(RENDER_EMBEDDED "an embedded platform with OpenGL ES (applications check RENDER_EMBEDDED)" ON)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" ON)
else()
  option(RENDER_EMBEDDED "an embedded platform with OpenGL ES (applications check RENDER_EMBEDDED)" OFF)
  option(NO_PTHREAD_CANCEL "Platform has no pthread cancel" OFF)
endif()
# Without threads the engine and its applications do their work in the main
# loop (the browser's main thread on the web).
if (EMSCRIPTEN AND NOT CGE_WEB_THREADS)
  set(ENGINE_SINGLE_THREADED_DEFAULT ON)
else()
  set(ENGINE_SINGLE_THREADED_DEFAULT OFF)
endif()
option(ENGINE_SINGLE_THREADED "build the engine and its applications without threads" ${ENGINE_SINGLE_THREADED_DEFAULT})

INCLUDE(image/CMakeLists.txt)
INCLUDE(input/CMakeLists.txt)
INCLUDE(io/CMakeLists.txt)
INCLUDE(math/CMakeLists.txt)
INCLUDE(mesh/CMakeLists.txt)
INCLUDE(platform/CMakeLists.txt)
INCLUDE(renderer/CMakeLists.txt)
INCLUDE(renderpass/CMakeLists.txt)
INCLUDE(system/CMakeLists.txt)
INCLUDE(ui/CMakeLists.txt)
INCLUDE(window/CMakeLists.txt)
INCLUDE(common/CMakeLists.txt)
INCLUDE(util/CMakeLists.txt)

# The engine is a shared library (CGE.dll / libCGE.so), so that applications
# link the LGPL code dynamically. Only what is marked CGE_API is exported.
# Browsers have no practical shared libraries, so on the web it is static
# (LGPL 2.1 section 6: the application's objects can be relinked instead).
if (EMSCRIPTEN)
	set(CGE_LIBRARY_TYPE STATIC)
else()
	set(CGE_LIBRARY_TYPE SHARED)
endif()
ADD_LIBRARY(Engine ${CGE_LIBRARY_TYPE}
	${Engine_image_SRCS}
	${Engine_input_SRCS}
	${Engine_io_SRCS}
	${Engine_math_SRCS}
	${Engine_mesh_SRCS}
	${Engine_platform_SRCS}
	${Engine_renderer_SRCS}
	${Engine_renderpass_SRCS}
	${Engine_system_SRCS}
	${Engine_ui_SRCS}
	${Engine_window_SRCS}
	${Engine_common_SRCS}
	${Engine_util_SRCS}

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
	${Engine_ui_HDRS}
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
if (CGE_LIBRARY_TYPE STREQUAL STATIC)
	# CGE_API and IMGUI_API are empty
	target_compile_definitions(Engine PUBLIC CGE_STATIC_DEFINE)
endif()
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
	# the engine and the application are two libraries that share STL objects
	if (NOT ANDROID_STL STREQUAL "c++_shared")
		message(FATAL_ERROR "The engine needs ANDROID_STL=c++_shared on Android (it is ${ANDROID_STL})")
	endif()
	target_compile_definitions(Engine PUBLIC UNIX ANDROID)
	target_link_libraries(Engine PUBLIC android log)
elseif (OS STREQUAL "web")
	# POSIX like Linux, but no X11 (__EMSCRIPTEN__ tells the web apart)
	target_compile_definitions(Engine PUBLIC UNIX)
	# applications are WebAssembly with a WebGL 2 context, a growing heap,
	# IndexedDB for the user folders (UserDirs) and the file system's runtime
	# functions for data packages loaded by the page (cge_web_package)
	target_link_options(Engine INTERFACE
		-sMIN_WEBGL_VERSION=2
		-sMAX_WEBGL_VERSION=2
		-sALLOW_MEMORY_GROWTH=1
		-sFORCE_FILESYSTEM=1
		-sEXPORTED_RUNTIME_METHODS=ccall
		-lidbfs.js
	)
	if (CGE_WEB_THREADS)
		# the applications' own objects need it too
		target_compile_options(Engine PUBLIC -pthread)
		# Engine::pumpFrame yields to the browser with emscripten_sleep; the
		# workers are created up front (a thread can't start one reliably);
		# a growing heap is slower for JavaScript code with threads, which
		# applications accept as guit3 does
		target_link_options(Engine INTERFACE -pthread -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -Wno-pthreads-mem-growth)
	endif()
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
	# Windows 10 is the minimum: its API (e.g. per-monitor DPI) is declared
	# and called directly
	target_compile_definitions(Engine PUBLIC _WIN32_WINNT=0x0A00 WINVER=0x0A00)
endif()
function(cge_engine_public_headers lib)
	target_include_directories(Engine PUBLIC $<TARGET_PROPERTY:${lib},INTERFACE_INCLUDE_DIRECTORIES>)
	target_compile_definitions(Engine PUBLIC $<TARGET_PROPERTY:${lib},INTERFACE_COMPILE_DEFINITIONS>)
endfunction()
cge_engine_public_headers(zlibstatic)
cge_engine_public_headers(minizip)

# Dear ImGui is compiled into the engine library (its sources come with the
# imgui target) and exported from it through ui/imgui_config.h, so the engine
# and the application share one ImGui context. Applications include <imgui.h>.
target_link_libraries(Engine PRIVATE imgui)
cge_engine_public_headers(imgui)
target_compile_definitions(Engine PUBLIC IMGUI_USER_CONFIG="ui/imgui_config.h")

# the render backends: OpenGL (renderer/GL/)
INCLUDE(renderer/GL/CMakeLists.txt)

if (RENDER_EMBEDDED)
	target_compile_definitions(Engine PUBLIC RENDER_EMBEDDED)
endif()

if (OS STREQUAL linux)
	target_link_libraries(Engine PUBLIC X11)
endif()

if (NO_PTHREAD_CANCEL)
	target_compile_definitions(Engine PRIVATE NO_PTHREAD_CANCEL)
endif()

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
SOURCE_GROUP("ui" FILES ${Engine_ui_SRCS})
SOURCE_GROUP("ui" FILES ${Engine_ui_HDRS})
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


# Shaders compiled into applications

set(CGE_EMBED_SHADERS_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/embed.cmake CACHE INTERNAL "")

# compiles the shader files (*.glsl, *.vert, *.frag) of a folder into a target
# as string literals, regenerated when one changes. The target gets a function
# void <function>(), declared in the generated header <function>.h; calling it
# makes the files loadable by ShaderLibrary under <prefix><file name>, e.g.
#   cge_embed_shaders(Game ${CMAKE_CURRENT_SOURCE_DIR}/shaders game/ registerGameShaders)
function(cge_embed_shaders target dir prefix function)
	file(GLOB files CONFIGURE_DEPENDS ${dir}/*.glsl ${dir}/*.vert ${dir}/*.frag)
	set(gendir ${CMAKE_CURRENT_BINARY_DIR}/${function})
	set(cpp ${gendir}/${function}.cpp)
	file(WRITE ${gendir}/${function}.h.in "// Generated by cge_embed_shaders. Don't edit.\n\n#pragma once\n\n//! makes the shader files of ${prefix} loadable by CGE::Gfx::ShaderLibrary\nvoid ${function}();\n")
	configure_file(${gendir}/${function}.h.in ${gendir}/${function}.h COPYONLY)
	add_custom_command(OUTPUT ${cpp}
		COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${dir} -DOUTPUT=${cpp} -DPREFIX=${prefix} -DFUNCTION=${function} -P ${CGE_EMBED_SHADERS_SCRIPT}
		DEPENDS ${CGE_EMBED_SHADERS_SCRIPT} ${files}
		COMMENT "Embedding the shaders of ${target}"
	)
	set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY ON)
	target_sources(${target} PRIVATE ${cpp} ${gendir}/${function}.h ${files})
	target_include_directories(${target} PUBLIC ${gendir})
	source_group("shaders" FILES ${files})
endfunction()


# Applications

# adds an application: an executable (add_executable's arguments, e.g.
#   cge_add_application(Game WIN32 main.cpp)
# ), but on Android a shared library, lib<target>.so, which the activity
# loads (android.app.lib_name in the manifest; see Engine/android). The
# application links CGE::Engine itself.
function(cge_add_application target)
	if (ANDROID)
		set(args ${ARGN})
		list(REMOVE_ITEM args WIN32 MACOSX_BUNDLE)
		add_library(${target} SHARED ${args})
	else()
		add_executable(${target} ${ARGN})
	endif()
endfunction()


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
	elseif (EMSCRIPTEN)
		# the engine is linked in; the page's other files go with its .html
		set(base $<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>)
		install(FILES ${base}.js ${base}.wasm DESTINATION ${destination} ${ARGN})
		install(FILES ${base}.data DESTINATION ${destination} OPTIONAL ${ARGN})
	else()
		install(TARGETS Engine LIBRARY DESTINATION ${destination}/lib ${ARGN})
		if (TARGET EngineGL)
			install(TARGETS EngineGL LIBRARY DESTINATION ${destination}/lib ${ARGN})
		endif()
		set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
	endif()
endfunction()


# Web pages

set(CGE_WEB_SHELL ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html CACHE INTERNAL "")

# splits a "<source>@<path>" argument of the web functions: the source
# relative to the current source folder becomes absolute (in spec_var), and
# the files it stands for (a folder's contents) are added to the list deps_var
function(_cge_web_file arg spec_var deps_var)
	string(FIND "${arg}" "@" at)
	if (at LESS 0)
		message(FATAL_ERROR "${arg}: web files are given as <source>@<path in the virtual file system>")
	endif()
	string(SUBSTRING "${arg}" 0 ${at} src)
	string(SUBSTRING "${arg}" ${at} -1 dest)
	get_filename_component(src "${src}" ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	set(${spec_var} "${src}${dest}" PARENT_SCOPE)
	if (IS_DIRECTORY "${src}")
		file(GLOB_RECURSE files CONFIGURE_DEPENDS "${src}/*")
	else()
		set(files "${src}")
	endif()
	set(${deps_var} ${${deps_var}} ${files} PARENT_SCOPE)
endfunction()

# makes an application a web page: <target>.html with the engine's shell page
# (the canvas over the whole browser window and a loading bar; SHELL names
# another one), <target>.js, <target>.wasm and, with PRELOAD, <target>.data
# with the given files, "<source>@<path in the virtual file system>"; a
# folder brings its contents. The application reads them as assets. Does
# nothing on other platforms.
#   cge_web_application(Game PRELOAD engine.ini@engine.ini assets@assets)
function(cge_web_application target)
	if (NOT EMSCRIPTEN)
		return()
	endif()
	cmake_parse_arguments(WEB "" "SHELL" "PRELOAD" ${ARGN})
	if (NOT WEB_SHELL)
		set(WEB_SHELL ${CGE_WEB_SHELL})
	endif()
	set_target_properties(${target} PROPERTIES SUFFIX ".html")
	target_link_options(${target} PRIVATE "SHELL:--shell-file \"${WEB_SHELL}\"")
	set(deps ${WEB_SHELL})
	foreach(arg ${WEB_PRELOAD})
		_cge_web_file("${arg}" spec deps)
		target_link_options(${target} PRIVATE "SHELL:--preload-file \"${spec}\"")
	endforeach()
	set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS ${deps})
endfunction()

# makes a data package that a web page loads before the application
# (?data=<name>, see web/shell.html): <name>.data and <name>.js in the
# current binary folder, with files given as for cge_web_application. They
# are made again when one of the files changes, and installed into
# DESTINATION (default: the installation's root) with COMPONENT, if given.
#   cge_web_package(demo ${CMAKE_SOURCE_DIR}/games/demo/data@data DESTINATION adventure)
function(cge_web_package name)
	if (NOT EMSCRIPTEN)
		return()
	endif()
	cmake_parse_arguments(WEB "" "DESTINATION;COMPONENT" "" ${ARGN})
	set(specs)
	set(deps)
	foreach(arg ${WEB_UNPARSED_ARGUMENTS})
		_cge_web_file("${arg}" spec deps)
		list(APPEND specs "${spec}")
	endforeach()
	set(out ${CMAKE_CURRENT_BINARY_DIR}/${name})
	add_custom_command(OUTPUT ${out}.data ${out}.js
		COMMAND ${EMSCRIPTEN_ROOT_PATH}/tools/file_packager${EMCC_SUFFIX} ${name}.data --quiet --preload ${specs} --js-output=${name}.js
		DEPENDS ${deps}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Packaging the web data ${name}"
	)
	add_custom_target(${name}_web_data ALL DEPENDS ${out}.data ${out}.js)
	if (NOT WEB_DESTINATION)
		set(WEB_DESTINATION .)
	endif()
	set(component)
	if (WEB_COMPONENT)
		set(component COMPONENT ${WEB_COMPONENT})
	endif()
	install(FILES ${out}.data ${out}.js DESTINATION ${WEB_DESTINATION} ${component})
endfunction()
INSTALL(FILES engine.ini DESTINATION Engine)
INSTALL(FILES textures/font/font.bmp DESTINATION Engine/textures/font)