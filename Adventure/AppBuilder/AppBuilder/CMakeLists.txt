###OPTIONS###
SET(OS windows CACHE STRING "the os to compile for")
SET(ARCH ${CPU})
OPTION(DISABLE_SOUND "disables the ffmpeg and openal backends" OFF)
OPTION(FFMPEG_OLD_API "activate use of older ffmpeg API" OFF)
OPTION(JNI_FRONTEND "enables a JNI frontend for Android" OFF)
OPTION(EMSCRIPTEN_FRONTEND "enables a C frontend for emscripten" OFF)
OPTION(ENGINE_SINGLE_THREADED "makes engine single threaded" ${EMSCRIPTEN_FRONTEND})
OPTION(DISABLE_EFX "disables EFX sound effects" ${EMSCRIPTEN_FRONTEND})

SET(AB_LINK_LIBS)

IF (OS STREQUAL windows)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF (OS STREQUAL windows)

IF(RENDER_EMBEDDED)
  ADD_DEFINITIONS(-DRENDER_TEGRA)
  IF(JNI_FRONTEND)
    SET(ADVENTURE_SRCS
      JNIFrontend.cpp
    )
	SET(AB_LINK_LIBS ${AB_LINK_LIBS} GLESv2)
  ELSEIF(EMSCRIPTEN_FRONTEND)
    SET(ADVENTURE_SRCS
      CFrontend.cpp
    )
	SET(AB_LINK_LIBS ${AB_LINK_LIBS} GLESv2)
  ELSE(JNI_FRONTEND)
    SET(ADVENTURE_SRCS 
      KDWindow.cpp
    )
    IF (OS STREQUAL windows)
      INCLUDE_DIRECTORIES($ENV{NV_WINGL_X86_PLAT}/include)
      LINK_DIRECTORIES($ENV{NV_WINGL_X86_PLAT}/lib/release)
      SET(AB_LINK_LIBS ${AB_LINK_LIBS} libnvkdmain libKD libGLES20)
    ELSEIF (OS STREQUAL qnx)
      SET(AB_LINK_LIBS ${AB_LINK_LIBS} nvkdmain KD GLESv2 EGL rt)
    ELSE (OS STREQUAL windows)
      ADD_DEFINITIONS(-DRASPBERRY_PI)
      SET(AB_LINK_LIBS ${AB_LINK_LIBS} GLESv2 EGL)
    ENDIF (OS STREQUAL windows)
  ENDIF(JNI_FRONTEND)

ELSE(RENDER_EMBEDDED)
  INCLUDE(FindOpenGL)
  SET(ADVENTURE_SRCS 
    CgeWindow.cpp
    CgeWindow.h
  )
  SET(AB_LINK_LIBS ${AB_LINK_LIBS} EngineMain ${OPENGL_gl_LIBRARY})
ENDIF(RENDER_EMBEDDED)

IF(ENGINE_SINGLE_THREADED)
ADD_DEFINITIONS(-DENGINE_SINGLE_THREADED)
ENDIF()

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
IF(NOT RENDER_EMBEDDED)
INCLUDE_DIRECTORIES(glew/include)
ADD_DEFINITIONS(-DGLEW_STATIC)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} glew)
ENDIF(NOT RENDER_EMBEDDED)

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
INCLUDE_DIRECTORIES(ffmpeg/include/win32)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/lib/${CPU})
ADD_LIBRARY(avformat STATIC IMPORTED)
SET_PROPERTY(TARGET avformat PROPERTY IMPORTED_LOCATION avformat.lib)
ADD_LIBRARY(avcodec STATIC IMPORTED)
SET_PROPERTY(TARGET avcodec PROPERTY IMPORTED_LOCATION avcodec.lib)
#ADD_LIBRARY(avdevice STATIC IMPORTED)
#SET_PROPERTY(TARGET avdevice PROPERTY IMPORTED_LOCATION avdevice.lib)
ADD_LIBRARY(avutil STATIC IMPORTED)
SET_PROPERTY(TARGET avutil PROPERTY IMPORTED_LOCATION avutil.lib)
ADD_LIBRARY(swscale STATIC IMPORTED)
SET_PROPERTY(TARGET swscale PROPERTY IMPORTED_LOCATION swscale.lib)
ADD_LIBRARY(swresample STATIC IMPORTED)
SET_PROPERTY(TARGET swresample PROPERTY IMPORTED_LOCATION swresample.lib)
ENDIF (OS STREQUAL windows)

IF (OS STREQUAL qnx)
INCLUDE_DIRECTORIES(ffmpeg/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/emscripten)
ENDIF (OS STREQUAL qnx)

IF (OS STREQUAL android)
INCLUDE_DIRECTORIES(ffmpeg/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/android)
ENDIF (OS STREQUAL android)

IF (OS STREQUAL linux)
#INCLUDE_DIRECTORIES(ffmpeg/include)
LINK_DIRECTORIES(/usr/local/lib)
ENDIF (OS STREQUAL linux)

SET(AB_LINK_LIBS ${AB_LINK_LIBS} avformat avcodec avutil swscale)
ELSE(NOT DISABLE_SOUND)
ADD_DEFINITIONS(-DDISABLE_SOUND)
ENDIF(NOT DISABLE_SOUND)
IF(FFMPEG_OLD_API)
ADD_DEFINITIONS(-DFFMPEG_OLD_API)
ELSE()
SET(AB_LINK_LIBS ${AB_LINK_LIBS} swresample)
ENDIF(FFMPEG_OLD_API)

###Engine###
INCLUDE_DIRECTORIES(../../../Engine)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} Engine ${ADD_LIB})

###AaventureBase###

IF(DISABLE_EFX)
ADD_DEFINITIONS(-DDISABLE_EFX)
ENDIF()

SET(AppBuilder_SRCS
  AdvDoc.cpp
  AdvDoc.h
  Animator.cpp
  Animator.h
  AST.cpp
  AST.h
  BlitObjects.cpp
  BlitObjects.h
  CIL.cpp
  CIL.h
  Engine.cpp
  Engine.h
  EngineObjects.cpp
  EngineObjects.h
  ExecutionContext.cpp
  ExecutionContext.h
  Font.cpp
  Font.h
  GuiRoom.h
  Ids.h
  Inventory.cpp
  Inventory.h
  ItemObject.cpp
  ItemObject.h
  Loader.cpp
  Loader.h
  Menu.cpp
  Menu.h
  Particles.cpp
  Particles.h
  PostProcessing.cpp
  PostProcessing.h
  Renderer.cpp
  Renderer.h
  SaveStateProvider.cpp
  SaveStateProvider.h
  Screenchange.cpp
  Screenchange.h
  Script.cpp
  Script.h
  ScriptDefs.h
  ScriptFunc.cpp
  ScriptFunc.h
  Sound.cpp
  Sound.h
  SoundPlayer.cpp
  SoundPlayer.h
  String.cpp
  String.h
  SwfPlayer.cpp
  SwfPlayer.h
  Textout.cpp
  Textout.h
  Vector.h
  ${PARSER_SRCS}
  pcdk.g
)

ADD_LIBRARY(AdventureBase STATIC ${AppBuilder_SRCS})

ADD_SUBDIRECTORY(lua)

###Adventure###

IF(JNI_FRONTEND)

SET(AB_LINK_LIBS ${AB_LINK_LIBS}
log
)

ADD_LIBRARY(Adventure SHARED
	${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

ADD_SUBDIRECTORY(java)

ELSEIF(EMSCRIPTEN_FRONTEND)

#ADD_DEFINITIONS(-sEXPORTED_FUNCTIONS=@functions.map) 

SET(ADV_SUFFIX .js)

ADD_EXECUTABLE(Adventure${ADV_SUFFIX}
	${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

SET_TARGET_PROPERTIES(Adventure${ADV_SUFFIX} PROPERTIES LINK_FLAGS "-O2 -s FULL_ES2=1 -s OUTLINING_LIMIT=10000 -s TOTAL_MEMORY=268435456 -s ASM_JS=1 -s EXPORTED_FUNCTIONS=\"['_main', '_fsMounted', '_fileWritten', '_fileSynced']\"")
#-s ALLOW_MEMORY_GROWTH=1

ELSE(JNI_FRONTEND)

ADD_EXECUTABLE(Adventure
  ${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

if(MSVC12)
SET_TARGET_PROPERTIES(Adventure PROPERTIES LINK_FLAGS "/SAFESEH:NO")
endif()

ENDIF(JNI_FRONTEND)

TARGET_LINK_LIBRARIES(Adventure${ADV_SUFFIX} AdventureBase ${AB_LINK_LIBS} ${ENGINE_LINK_LIBRARIES})

IF (NOT RENDER_EMBEDDED)
SET_TARGET_PROPERTIES(Adventure
PROPERTIES
WIN32_EXECUTABLE TRUE)
ENDIF(NOT RENDER_EMBEDDED)

ADD_SUBDIRECTORY(eax2efx)

if (EMSCRIPTEN_FRONTEND)
add_subdirectory(emscripten)
endif()

add_custom_command(TARGET Adventure${ADV_SUFFIX} POST_BUILD
COMMAND ${Java_JAR_EXECUTABLE} cfM engine1.dat -C $<TARGET_FILE_DIR:Adventure${ADV_SUFFIX}> $<TARGET_FILE_NAME:Adventure${ADV_SUFFIX}>)

#install

INSTALL(TARGETS Adventure${ADV_SUFFIX} RUNTIME DESTINATION adventure COMPONENT adv_runtime)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine1.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)

if (OS STREQUAL windows)
#openal
get_filename_component(OPENAL_PATH ${OPENAL_LIBRARY} PATH)
set(OPENAL_FILE OpenAL32.dll)
set(OPENAL_DLL "${OPENAL_PATH}/${OPENAL_FILE}")
message(STATUS "OpenAL: ${OPENAL_DLL}")

add_custom_command(OUTPUT engine2.dat
COMMAND ${Java_JAR_EXECUTABLE} cfM engine2.dat -C ${OPENAL_PATH} ${OPENAL_FILE}
-C ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU} avcodec-55.dll
-C ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU} avformat-55.dll
-C ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU} avutil-52.dll
-C ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU} swresample-0.dll
-C ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU} swscale-2.dll
-C ${CMAKE_CURRENT_SOURCE_DIR}/../../../Engine/lib/${CPU} d3dcompiler_47.dll
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avcodec-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avformat-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avutil-52.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swresample-0.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swscale-2.dll
${CMAKE_CURRENT_SOURCE_DIR}/../../../Engine/lib/${CPU}/d3dcompiler_47.dll
)

add_custom_target(pack_engine_deps ALL DEPENDS engine2.dat)

INSTALL(FILES ${OPENAL_DLL} DESTINATION adventure)

#ffmpeg
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avcodec-55.dll 
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avformat-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avutil-52.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swresample-0.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swscale-2.dll
${CMAKE_CURRENT_SOURCE_DIR}/../../../Engine/lib/${CPU}/d3dcompiler_47.dll
DESTINATION adventure)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine2.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
endif()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/engine.ini DESTINATION adventure)

#packaging
#set(CPACK_GENERATOR "ZIP")
#set(CPACK_PACKAGE_NAME "engine1.dat")
#set(CPACK_COMPONENTS_ALL adv_runtime)
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "StoryDesigner")
#INCLUDE(CPack)