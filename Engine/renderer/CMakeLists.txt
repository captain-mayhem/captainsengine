SET(PRE renderer)

IF(RENDER_DIRECTX)
	SET(RENDERER_SRCS
		${PRE}/DirectX/DXindexbuffer.cpp
		${PRE}/DirectX/DXrenderer.cpp
		${PRE}/DirectX/DXshader.cpp
		${PRE}/DirectX/DXtexture.cpp
		${PRE}/DirectX/DXvertexbuffer.cpp
		${RENDERER_SRCS}
	)
	SET(RENDERER_HDRS_DX
		${PRE}/DirectX/DXindexbuffer.h
		${PRE}/DirectX/DXrenderer.h
		${PRE}/DirectX/DXshader.h
		${PRE}/DirectX/DXtexture.h
		${PRE}/DirectX/DXvertexbuffer.h
	)
ENDIF(RENDER_DIRECTX)
IF(RENDER_OPENGL)
	SET(RENDERER_SRCS
		${PRE}/OpenGL/OGLindexbuffer.cpp
		${PRE}/OpenGL/OGLrenderer.cpp
		${PRE}/OpenGL/OGLtexture.cpp
		${PRE}/OpenGL/OGLvertexbuffer.cpp
		${RENDERER_SRCS}
	)
	SET(RENDERER_HDRS_OGL
		${PRE}/OpenGL/OGLindexbuffer.h
		${PRE}/OpenGL/OGLrenderer.h
		${PRE}/OpenGL/OGLtexture.h
		${PRE}/OpenGL/OGLvertexbuffer.h
	)
ENDIF(RENDER_OPENGL)
IF(RENDER_GLES)
	SET(RENDERER_SRCS
		${PRE}/GLES/GLESindexbuffer.cpp
		${PRE}/GLES/GLESrenderer.cpp
		${PRE}/GLES/GLEStexture.cpp
		${PRE}/GLES/GLESvertexbuffer.cpp
		${RENDERER_SRCS}
	)
	SET(RENDERER_HDRS_GLES
		${PRE}/GLES/GLESindexbuffer.h
		${PRE}/GLES/GLESrenderer.h
		${PRE}/GLES/GLEStexture.h
		${PRE}/GLES/GLESvertexbuffer.h
	)
ENDIF(RENDER_GLES)
IF(RENDER_GL2 OR RENDER_EMBEDDED)
	SET(RENDERER_SRCS
		${RENDERER_SRCS}
		${PRE}/GL2/GL2Renderer.cpp
		${PRE}/GL2/GL2Shader.cpp
		${PRE}/GL2/GL2indexbuffer.cpp
		${PRE}/GL2/GL2vertexbuffer.cpp
	)
	SET(RENDERER_HDRS_GL2
		${PRE}/GL2/GL2Renderer.h
		${PRE}/GL2/GL2Shader.h
		${PRE}/GL2/GL2indexbuffer.h
		${PRE}/GL2/GL2vertexbuffer.h
	)
ENDIF(RENDER_GL2 OR RENDER_EMBEDDED)

SET(Engine_renderer_SRCS
	${PRE}/camera.cpp
	${PRE}/font.cpp
	${PRE}/forms.cpp
	${PRE}/indexbuffer.cpp
	${PRE}/light.cpp
	${PRE}/material.cpp
	${PRE}/model.cpp
	${PRE}/renderer.cpp
	${PRE}/scene.cpp
	${PRE}/shader.cpp
	${PRE}/texture.cpp
	${PRE}/vertexbuffer.cpp
	${RENDERER_SRCS}
)

SET(RENDERER_HDRS_CORE
	${PRE}/camera.h
	${PRE}/dummyrenderer.h
	${PRE}/font.h
	${PRE}/forms.h
	${PRE}/indexbuffer.h
	${PRE}/light.h
	${PRE}/material.h
	${PRE}/model.h
	${PRE}/renderer.h
	${PRE}/scene.h
	${PRE}/shader.h
	${PRE}/texture.h
	${PRE}/vertexbuffer.h
)

INSTALL(FILES ${RENDERER_HDRS_CORE} DESTINATION Engine/include/${PRE} COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_DX} DESTINATION Engine/include/${PRE}/DirectX COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_OGL} DESTINATION Engine/include/${PRE}/OpenGL COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_GLES} DESTINATION Engine/include/${PRE}/GLES COMPONENT CGE)
INSTALL(FILES ${RENDERER_HDRS_GL2} DESTINATION Engine/include/${PRE}/GL2 COMPONENT CGE)

SET(Engine_renderer_HDRS
	${RENDERER_HDRS_CORE}
	${RENDERER_HDRS_DX}
	${RENDERER_HDRS_OGL}
	${RENDERER_HDRS_GLES}
	${RENDERER_HDRS_GL2}
)
