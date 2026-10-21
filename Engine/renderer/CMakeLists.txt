SET(PRE renderer)

IF(RENDER_OPENGL)
	SET(RENDERER_SRCS
		${PRE}/OpenGL/OGLfunctions.cpp
		${PRE}/OpenGL/OGLindexbuffer.cpp
		${PRE}/OpenGL/OGLrenderer.cpp
		${PRE}/OpenGL/OGLtexture.cpp
		${PRE}/OpenGL/OGLvertexbuffer.cpp
		${RENDERER_SRCS}
	)
	SET(RENDERER_HDRS_OGL
		${PRE}/OpenGL/OGLfunctions.h
		${PRE}/OpenGL/OGLindexbuffer.h
		${PRE}/OpenGL/OGLrenderer.h
		${PRE}/OpenGL/OGLtexture.h
		${PRE}/OpenGL/OGLvertexbuffer.h
	)
ENDIF(RENDER_OPENGL)
IF(RENDER_GL2 OR RENDER_EMBEDDED)
	SET(RENDERER_SRCS
		${RENDERER_SRCS}
		${PRE}/GL2/GL2Renderer.cpp
		${PRE}/GL2/GL2rendertarget.cpp
		${PRE}/GL2/GL2Shader.cpp
		${PRE}/OpenGL/OGLfunctions.cpp
		${PRE}/OpenGL/OGLtexture.cpp
		${PRE}/GL2/GL2indexbuffer.cpp
		${PRE}/GL2/GL2vertexbuffer.cpp
	)
	SET(RENDERER_HDRS_GL2
		${PRE}/GL2/GL2Renderer.h
		${PRE}/GL2/GL2rendertarget.h
		${PRE}/GL2/GL2Shader.h
		${PRE}/OpenGL/OGLfunctions.h
		${PRE}/OpenGL/OGLtexture.h
		${PRE}/GL2/GL2indexbuffer.h
		${PRE}/GL2/GL2vertexbuffer.h
	)
ENDIF(RENDER_GL2 OR RENDER_EMBEDDED)

SET(Engine_renderer_SRCS
	${PRE}/camera.cpp
	${PRE}/drawcontext.cpp
	${PRE}/font.cpp
	${PRE}/forms.cpp
	${PRE}/indexbuffer.cpp
	${PRE}/light.cpp
	${PRE}/material.cpp
	${PRE}/nulldevice.cpp
	${PRE}/renderdevice.cpp
	${PRE}/renderer.cpp
	${PRE}/rendertarget.cpp
	${PRE}/scene.cpp
	${PRE}/scenenode.cpp
	${PRE}/shader.cpp
	${PRE}/shaderlibrary.cpp
	${PRE}/texture.cpp
	${PRE}/uniformstream.cpp
	${PRE}/vertexbuffer.cpp
	${RENDERER_SRCS}
)

SET(RENDERER_HDRS_CORE
	${PRE}/camera.h
	${PRE}/drawcontext.h
	${PRE}/dummyrenderer.h
	${PRE}/font.h
	${PRE}/forms.h
	${PRE}/gfxtypes.h
	${PRE}/indexbuffer.h
	${PRE}/light.h
	${PRE}/material.h
	${PRE}/nulldevice.h
	${PRE}/renderbackend.h
	${PRE}/renderdevice.h
	${PRE}/renderer.h
	${PRE}/rendertarget.h
	${PRE}/scene.h
	${PRE}/scenenode.h
	${PRE}/shader.h
	${PRE}/shaderlibrary.h
	${PRE}/texture.h
	${PRE}/uniformstream.h
	${PRE}/vertexbuffer.h
)

INSTALL(FILES ${RENDERER_HDRS_CORE} DESTINATION Engine/include/${PRE} COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_OGL} DESTINATION Engine/include/${PRE}/OpenGL COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_GLES} DESTINATION Engine/include/${PRE}/GLES COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_GL2} DESTINATION Engine/include/${PRE}/GL2 COMPONENT CGE)

# The built-in shaders (Engine/shaders/) are compiled into the engine library
# as string literals, regenerated when a shader file changes.
file(GLOB CGE_SHADER_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
)
set(CGE_BUILTIN_SHADERS_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated/builtinshaders.cpp)
add_custom_command(OUTPUT ${CGE_BUILTIN_SHADERS_CPP}
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders -DOUTPUT=${CGE_BUILTIN_SHADERS_CPP} -P ${CMAKE_CURRENT_SOURCE_DIR}/shaders/embed.cmake
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/embed.cmake ${CGE_SHADER_FILES}
	COMMENT "Embedding the built-in shaders"
)
set_source_files_properties(${CGE_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY ON)
SET(Engine_renderer_SRCS
	${Engine_renderer_SRCS}
	${CGE_BUILTIN_SHADERS_CPP}
	${CGE_SHADER_FILES}
	${PRE}/builtinshaders.h
)
SOURCE_GROUP("shaders" FILES ${CGE_SHADER_FILES})

SET(Engine_renderer_HDRS
	${RENDERER_HDRS_CORE}
	${RENDERER_HDRS_OGL}
	${RENDERER_HDRS_GLES}
	${RENDERER_HDRS_GL2}
)
