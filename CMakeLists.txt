PROJECT(CaptainsEngine)

OPTION(DIRECTX "compiles with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)

SET(SDK_DIR "E:/Program Files/Windows CE Tools/wce500/STANDARDSDK_500")
SET(VC_DIR "$ENV{VC_INSTALL_DIR}")

IF (${OS} STREQUAL "")
  MESSAGE(ERROR "OS must be set to the current operating system")
ENDIF (${OS} STREQUAL "")
SET(OS ${OS} CACHE STRING "the current operating system")

INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/include)

IF (${OS} STREQUAL "windows")
  MESSAGE(STATUS "Compiling for Windows")
  ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
ENDIF (${OS} STREQUAL "windows")
IF (${OS} STREQUAL "linux")
  MESSAGE(STATUS "Compiling for Linux")
  ADD_DEFINITIONS(-DUNIX)
ENDIF (${OS} STREQUAL "linux")
IF (${OS} STREQUAL "wince")
  MESSAGE(STATUS "Cross compiling for Windows CE")
  SET(CMAKE_CXX_COMPILER "${VC_DIR}/ce/bin/x86_arm/cl.exe")
  SET(CMAKE_C_COMPILER "${VC_DIR}/ce/bin/x86_arm/cl.exe")
  
  SET(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od")
  SET(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od")
  SET(COMMON_FLAGS "/D_NODIRECTX /DBUFSIZ=512 /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /W3 /Zm1000 /X /DARM /D_ARM_ /DUNDER_CE /D_WIN32_WCE=0x500 /I\"${VC_DIR}/ce/include\" /I\"${SDK_DIR}/Include/Armv4i\"")
  SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} /EHsc /GR")
  SET(CMAKE_C_FLAGS "${COMMON_FLAGS}")
  ADD_DEFINITIONS(-DHAVE_BOOLEAN)

  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "msvcrtd.lib")
  SET(CMAKE_EXE_LINKER_FLAGS "coredll.lib corelibc.lib /MACHINE:THUMB /NODEFAULTLIB /LIBPATH:\"${SDK_DIR}/LIB/ARMV4I\" /LIBPATH:\"${VC_DIR}/ce/lib/armv4i\"")
ENDIF (${OS} STREQUAL "wince")

ADD_SUBDIRECTORY(extern)
ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(editor)
ADD_SUBDIRECTORY(heroquest)
ADD_SUBDIREcTORY(AdventureGui)
