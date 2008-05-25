PROJECT(CaptainsEngine)

OPTION(DIRECTX "compiles with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)

IF (${OS} STREQUAL "")
  MESSAGE(ERROR "OS must be set to the current operating system")
ENDIF (${OS} STREQUAL "")
SET(OS ${OS} CACHE STRING "the current operating system")

INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/include)

IF (${OS} STREQUAL "windows")
  ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
ENDIF (${OS} STREQUAL "windows")
IF (${OS} STREQUAL "linux")
  ADD_DEFINITIONS(-DUNIX)
ENDIF (${OS} STREQUAL "linux")

ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(editor)
ADD_SUBDIRECTORY(heroquest)
ADD_SUBDIREcTORY(AdventureGui)
