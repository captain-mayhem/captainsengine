###OPTIONS###
SET(OS windows CACHE STRING "the os to compile for")
SET(ARCH ${CPU})
OPTION(DISABLE_SOUND "disables the ffmpeg and openal backends" OFF)
OPTION(TEGRA_RENDERER "switches the renderer to OpenKode/GLES2 rendering" OFF)

SET(AB_LINK_LIBS)

IF (OS STREQUAL windows)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF (OS STREQUAL windows)

IF(TEGRA_RENDERER)
ADD_DEFINITIONS(-DRENDER_TEGRA)
ENDIF(TEGRA_RENDERER)

###WXWIDGETS###

#INCLUDE(FindwxWidgets)

#SET(wxWidgets_USE_LIBS base core gl media)
#FIND_PACKAGE(wxWidgets REQUIRED base core gl media)
#INCLUDE(${wxWidgets_USE_FILE})
#ADD_DEFINITIONS(-DwxUSE_NO_MANIFEST=1)
#SET(AB_LINK_LIBS ${AB_LINK_LIBS} ${wxWidgets_LIBRARIES})

###ANTLR###

FILE(GLOB ANTLR_SRCS antlr/src/*.c)
FILE(GLOB ANTLR_HDRS antlr/include/*.h)
ADD_LIBRARY(antlr STATIC
  ${ANTLR_SRCS}
  ${ANTLR_HDRS}
)

INCLUDE_DIRECTORIES(
antlr/include
)

INCLUDE(FindJava)

SET(PARSER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/pcdkParser.c
${CMAKE_CURRENT_SOURCE_DIR}/pcdkParser.h
${CMAKE_CURRENT_SOURCE_DIR}/pcdkLexer.c
${CMAKE_CURRENT_SOURCE_DIR}/pcdkLexer.h
)

SET_SOURCE_FILES_PROPERTIES(
${PARSER_SRCS}
PROPERTIES
GENERATED TRUE
LANGUAGE CXX
)

ADD_CUSTOM_COMMAND(OUTPUT ${PARSER_SRCS}
COMMAND ${JAVA_RUNTIME} -cp antlrworks-1.2.3.jar org.antlr.Tool pcdk.g
DEPENDS pcdk.g
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "antlr'ing grammar"
)

SOURCE_GROUP("Resource Files" FILES AppBuilder.rc)
SOURCE_GROUP("Grammar" FILES pcdk.g)

SET(AB_LINK_LIBS ${AB_LINK_LIBS} antlr)

###GLEW###

INCLUDE_DIRECTORIES(glew/include)
ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_SUBDIRECTORY(glew)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} glew)

IF(NOT DISABLE_SOUND)
###Open AL###
INCLUDE(FindOpenAL)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
SET(AB_LINK_LIBS ${AB_LINK_LIBS} ${OPENAL_LIBRARY})

###ALUT###
INCLUDE_DIRECTORIES(alut/include)
#this crazy define prevents us from getting declspec dllexport/import defined
ADD_DEFINITIONS(-D_XBOX)
ADD_SUBDIRECTORY(alut)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} alut)

###FFMPEG###
IF (OS STREQUAL windows)
INCLUDE_DIRECTORIES(ffmpeg/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/lib)
ADD_LIBRARY(avformat STATIC IMPORTED)
SET_PROPERTY(TARGET avformat PROPERTY IMPORTED_LOCATION avformat-52.lib)
ADD_LIBRARY(avcodec STATIC IMPORTED)
SET_PROPERTY(TARGET avcodec PROPERTY IMPORTED_LOCATION avcodec-52.lib)
ADD_LIBRARY(avdevice STATIC IMPORTED)
SET_PROPERTY(TARGET avdevice PROPERTY IMPORTED_LOCATION avdevice-52.lib)
ADD_LIBRARY(avutil STATIC IMPORTED)
SET_PROPERTY(TARGET avutil PROPERTY IMPORTED_LOCATION avutil-49.lib)
ADD_LIBRARY(swscale STATIC IMPORTED)
SET_PROPERTY(TARGET swscale PROPERTY IMPORTED_LOCATION swscale-0.lib)
ENDIF (OS STREQUAL windows)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} avformat avcodec avdevice avutil swscale)
ELSE(NOT DISABLE_SOUND)
ADD_DEFINITIONS(-DDISABLE_SOUND)
ENDIF(NOT DISABLE_SOUND)

###Engine###
INCLUDE_DIRECTORIES(../../../Engine)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} Engine)

###AaventureBase###

SET(AppBuilder_SRCS
  AdvDoc.cpp
  AdvDoc.h
  Animator.cpp
  Animator.h
  AST.h
  BlitObjects.cpp
  BlitObjects.h
  CIL.cpp
  CIL.h
  Engine.cpp
  Engine.h
  EngineObjects.cpp
  EngineObjects.h
  Font.cpp
  Font.h
  Ids.h
  Inventory.cpp
  Inventory.h
  Renderer.cpp
  Renderer.h
  SaveStateProvider.cpp
  SaveStateProvider.h
  Script.cpp
  Script.h
  ScriptDefs.h
  ScriptFunc.cpp
  ScriptFunc.h
  Sound.cpp
  Sound.h
  Vector.h
  ${PARSER_SRCS}
  pcdk.g
)

ADD_LIBRARY(AdventureBase STATIC ${AppBuilder_SRCS})

###Adventure###

ADD_EXECUTABLE(Adventure
  CgeWindow.cpp
  CgeWindow.h
)
TARGET_LINK_LIBRARIES(Adventure AdventureBase ${AB_LINK_LIBS} EngineMain)

SET_TARGET_PROPERTIES(Adventure
PROPERTIES
WIN32_EXECUTABLE TRUE)