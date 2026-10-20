# The OpenGL render backend (included by Engine/CMakeLists.txt after the
# Engine target). On desktop it is a plugin library, CGE_GL.dll /
# libCGE_GL.so, next to the engine library, which RenderDevice::create loads.
# On Android it is compiled into the engine (CGE_GL_BUILTIN).

set(CGE_GL_SRCS
	renderer/GL/GLContextEGLAndroid.cpp
	renderer/GL/GLContextEGLX11.cpp
	renderer/GL/GLContextWGL.cpp
	renderer/GL/GLDevice.cpp
)
set(CGE_GL_HDRS
	renderer/GL/GLContext.h
	renderer/GL/GLDevice.h
)

if (OS STREQUAL "android")
	target_sources(Engine PRIVATE ${CGE_GL_SRCS} ${CGE_GL_HDRS})
	target_compile_definitions(Engine PRIVATE CGE_GL_BUILTIN)
	SOURCE_GROUP("renderer/GL" FILES ${CGE_GL_SRCS} ${CGE_GL_HDRS})
else()
	add_library(EngineGL MODULE ${CGE_GL_SRCS} ${CGE_GL_HDRS} renderer/GL/GLPlugin.cpp)
	add_library(CGE::GL ALIAS EngineGL)
	set_target_properties(EngineGL PROPERTIES
		OUTPUT_NAME CGE_GL
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
	# the plugin's own copy of glad, loaded from its context
	target_link_libraries(EngineGL PRIVATE Engine glad)
	if (OS STREQUAL "linux")
		# EGL on X11; the GL functions come from eglGetProcAddress
		find_package(OpenGL REQUIRED COMPONENTS EGL)
		target_link_libraries(EngineGL PRIVATE OpenGL::EGL X11)
		# glad stays private to the plugin, like the engine's copy
		target_link_options(EngineGL PRIVATE -Wl,--exclude-libs,ALL)
	else()
		# WGL
		find_package(OpenGL REQUIRED)
		target_link_libraries(EngineGL PRIVATE OpenGL::GL)
	endif()
	SOURCE_GROUP("" FILES ${CGE_GL_SRCS} ${CGE_GL_HDRS} renderer/GL/GLPlugin.cpp)
endif()
