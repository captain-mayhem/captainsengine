PROJECT(CaptainsEngine)

OPTION(RENDER_OPENGL "compile renderer with OpenGL support" ON)
OPTION(RENDER_DIRECTX "compiles with DirectX libraries (Windows only) in order to be able to use this renderer" OFF)
OPTION(RENDER_GLES "compile renderer with OpenGL ES support" OFF)

SET(VC_DIR "$ENV{VCINSTALLDIR}")

IF (NOT OS)
  MESSAGE(ERROR "OS must be set to the desired operating system")
ENDIF (NOT OS)
SET(OS ${OS} CACHE STRING "the current operating system")
IF (NOT CPU)
  MESSAGE(ERROR "CPU must be set to the desired processor architecture")
ENDIF (NOT CPU)
SET(CPU ${CPU} CACHE STRING "the current processor architecture")

SET(ADD_LIB ${ADD_LIB} CACHE STRING "additional libraries to be linked against")
SET(ENGINE_LINK_LIBRARIES "")

INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/include)

IF (${OS} STREQUAL "wince")
  IF (NOT SDK_DIR)
    MESSAGE(ERROR "SDK_DIR must be set for Windows CE build")
  ENDIF (NOT SDK_DIR)
  SET(SDK_DIR ${SDK_DIR} CACHE STRING "the path to the Windows CE SDK")
  IF (NOT SDK_LIB_SUFFIX)
    MESSAGE(ERROR "SDK_LIB_SUFFIX must be set for Windows CE build")
  ENDIF (NOT SDK_LIB_SUFFIX)
  SET(SDK_LIB_SUFFIX ${SDK_LIB_SUFFIX} CACHE STRING "the library path suffix of the Windows CE SDK")
  IF (NOT SDK_INC_SUFFIX)
    MESSAGE(ERROR "SDK_INC_SUFFIX must be set for Windows CE build")
  ENDIF (NOT SDK_INC_SUFFIX)
  SET(SDK_INC_SUFFIX ${SDK_INC_SUFFIX} CACHE STRING "the include path suffix of the Windows CE SDK")
ENDIF (${OS} STREQUAL "wince")
IF (${OS} STREQUAL "windows")
  MESSAGE(STATUS "Compiling for Windows")
  ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
ENDIF (${OS} STREQUAL "windows")
IF (${OS} STREQUAL "linux")
  MESSAGE(STATUS "Compiling for Linux")
  ADD_DEFINITIONS(-DUNIX)
  SET(ENGINE_LINK_LIBRARIES
    X11
    Xxf86vm
    pthread
    ${ENGINE_LINK_LIBRARIES}
  )
ENDIF (${OS} STREQUAL "linux")
IF (${OS} STREQUAL "wince")
  MESSAGE(STATUS "Cross compiling for Windows CE")
  SET(CMAKE_CXX_COMPILER "${VC_DIR}/ce/bin/x86_arm/cl.exe")
  SET(CMAKE_C_COMPILER "${VC_DIR}/ce/bin/x86_arm/cl.exe")
  
  SET(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od")
  SET(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od")
  SET(COMMON_FLAGS "/DBUFSIZ=512 /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /W3 /Zm1000 /X /DARM /D_ARM_ /DUNDER_CE /D_WIN32_WCE=0x500 /I\"${VC_DIR}/ce/include\" /I\"${SDK_DIR}/${SDK_INC_SUFFIX}\"")
  SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} /EHsc /GR")
  SET(CMAKE_C_FLAGS "${COMMON_FLAGS}")
  ADD_DEFINITIONS(-DHAVE_BOOLEAN)

  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "libcpmtd.lib libcmtd.lib ${ADD_LIB}")
  SET(CMAKE_EXE_LINKER_FLAGS "coredll.lib corelibc.lib /MACHINE:ARM /SUBSYSTEM:WINDOWSCE /ARMPADCODE /NODEFAULTLIB /LIBPATH:\"${SDK_DIR}/${SDK_LIB_SUFFIX}\" /LIBPATH:\"${VC_DIR}/ce/lib/armv4i\"")
  SET(CMAKE_CREATE_CONSOLE_EXE "")
  SET(CMAKE_CREATE_WIN32_EXE "")
ENDIF (${OS} STREQUAL "wince")

SET(ENGINE_LINK_LIBRARIES
	jpeg
	lua
	lualib
	${ENGINE_LINK_LIBRARIES}
)

IF(RENDER_DIRECTX)
	ADD_DEFINITIONS(-DDIRECTX)
	SET(ENGINE_LINK_LIBRARIES
		d3d9
		d3dx9
		${ENGINE_LINK_LIBRARIES}
	)
ENDIF(RENDER_DIRECTX)
IF(RENDER_OPENGL)
	INCLUDE(FindOpenGL)
	ADD_DEFINITIONS(-DOPENGL)
	SET(ENGINE_LINK_LIBRARIES
		${OPENGL_LIBRARIES}
		${ENGINE_LINK_LIBRARIES}
	)
ENDIF(RENDER_OPENGL)
IF(RENDER_GLES)
	ADD_DEFINITIONS(-DGLES)
ENDIF(RENDER_GLES)


ADD_SUBDIRECTORY(extern)
ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(editor)
ADD_SUBDIRECTORY(heroquest)
ADD_SUBDIREcTORY(AdventureGui)
