SET(PRE renderer)

SET(Engine_renderer_SRCS
	${PRE}/camera.cpp
	${PRE}/drawcontext.cpp
	${PRE}/font.cpp
	${PRE}/forms.cpp
	${PRE}/light.cpp
	${PRE}/material.cpp
	${PRE}/nulldevice.cpp
	${PRE}/renderdevice.cpp
	${PRE}/scene.cpp
	${PRE}/scenenode.cpp
	${PRE}/shaderlibrary.cpp
	${PRE}/textureloader.cpp
	${PRE}/uniformstream.cpp
)

SET(RENDERER_HDRS_CORE
	${PRE}/camera.h
	${PRE}/drawcontext.h
	${PRE}/font.h
	${PRE}/forms.h
	${PRE}/gfxtypes.h
	${PRE}/light.h
	${PRE}/material.h
	${PRE}/nulldevice.h
	${PRE}/renderbackend.h
	${PRE}/renderdevice.h
	${PRE}/scene.h
	${PRE}/scenenode.h
	${PRE}/shaderlibrary.h
	${PRE}/textureloader.h
	${PRE}/uniformstream.h
)

INSTALL(FILES ${RENDERER_HDRS_CORE} DESTINATION Engine/include/${PRE} COMPONENT CGE)

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
)
