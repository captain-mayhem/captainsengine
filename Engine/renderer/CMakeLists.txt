SET(PRE renderer)

SET(Engine_renderer_SRCS
	${PRE}/DXrenderer.cpp
	${PRE}/DXtexture.cpp
	${PRE}/DXvertexbuffer.cpp
	${PRE}/font.cpp
	${PRE}/forms.cpp
	${PRE}/material.cpp
	${PRE}/model.cpp
	${PRE}/OGLrenderer.cpp
	${PRE}/OGLtexture.cpp
	${PRE}/OGLvertexbuffer.cpp
	${PRE}/renderer.cpp
	${PRE}/scene.cpp
	${PRE}/texture.cpp
	${PRE}/vertexbuffer.cpp
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