PROJECT(HeroEngine)

OPTION(DIRECTX "compiles with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)

INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/include)
ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)

ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(editor)
ADD_SUBDIRECTORY(heroquest)