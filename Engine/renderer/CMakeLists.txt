SET(PRE renderer)

IF(RENDER_DIRECTX)
	SET(RENDERER_SRCS
		${PRE}/DXrenderer.cpp
		${PRE}/DXtexture.cpp
		${PRE}/DXvertexbuffer.cpp
		${RENDERER_SRCS}
	)
ENDIF(RENDER_DIRECTX)
IF(RENDER_OPENGL)
	SET(RENDERER_SRCS
		${PRE}/OGLrenderer.cpp
		${PRE}/OGLtexture.cpp
		${PRE}/OGLvertexbuffer.cpp
		${RENDERER_SRCS}
	)
ENDIF(RENDER_OPENGL)

SET(Engine_renderer_SRCS
	${PRE}/font.cpp
	${PRE}/forms.cpp
	${PRE}/material.cpp
	${PRE}/model.cpp
	${PRE}/renderer.cpp
	${PRE}/scene.cpp
	${PRE}/texture.cpp
	${PRE}/vertexbuffer.cpp
	${RENDERER_SRCS}
)

SET(Engine_renderer_HDRS
	${PRE}/dummyrenderer.h
	${PRE}/DXrenderer.h
	${PRE}/DXtexture.h
	${PRE}/DXvertexbuffer.h
	${PRE}/font.h
	${PRE}/forms.h
	${PRE}/material.h
	${PRE}/model.h
	${PRE}/OGLrenderer.h
	${PRE}/OGLtexture.h
	${PRE}/OGLvertexbuffer.h
	${PRE}/renderer.h
	${PRE}/scene.h
	${PRE}/texture.h
	${PRE}/vertexbuffer.h
)

INSTALL(FILES ${Engine_renderer_HDRS} DESTINATION Engine/include/${PRE})