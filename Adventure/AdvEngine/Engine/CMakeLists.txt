###OPTIONS###
# The sound code uses the FFmpeg 2.x API, which only the bundled Windows
# libraries provide (current Linux distributions ship FFmpeg 5+, the web has
# none). It moves to the engine's audio module later; until then Linux and the
# web build without sound.
if (OS STREQUAL "linux" OR OS STREQUAL "web")
  set(DISABLE_SOUND_DEFAULT ON)
else()
  set(DISABLE_SOUND_DEFAULT OFF)
endif()
OPTION(DISABLE_SOUND "disables the ffmpeg and openal backends" ${DISABLE_SOUND_DEFAULT})
OPTION(FFMPEG_OLD_API "activate use of older ffmpeg API" OFF)
OPTION(JNI_FRONTEND "enables a JNI frontend for Android" OFF)
OPTION(DISABLE_EFX "disables EFX sound effects" ${EMSCRIPTEN})

# the frontend (window and main loop) and the libraries only it needs
SET(ADVENTURE_LINK_LIBS)
IF(RENDER_EMBEDDED)
  IF(JNI_FRONTEND)
    SET(ADVENTURE_SRCS JNIFrontend.cpp)
    SET(ADVENTURE_LINK_LIBS GLESv2 log)
  ELSE()
    SET(ADVENTURE_SRCS CgeWindow.cpp CgeWindow.h)
    SET(ADVENTURE_LINK_LIBS OpenSLES)
  ENDIF()
ELSE(RENDER_EMBEDDED)
  # draws through the engine's render device, without own GL calls; also
  # the web's frontend (the engine's window and main loop in the browser)
  SET(ADVENTURE_SRCS CgeWindow.cpp CgeWindow.h)
ENDIF(RENDER_EMBEDDED)

###ANTLR###

FILE(GLOB ANTLR_SRCS antlr/src/*.c)
FILE(GLOB ANTLR_HDRS antlr/include/*.h)
ADD_LIBRARY(antlr STATIC
  ${ANTLR_SRCS}
  ${ANTLR_HDRS}
)
target_include_directories(antlr PUBLIC antlr/include)
if (NOT WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  # on Windows antlr3defs.h detects 64-bit from _WIN64; elsewhere its configure
  # script would define this, otherwise pointers are stored in 32-bit markers
  target_compile_definitions(antlr PUBLIC ANTLR3_USE_64BIT)
endif()
if (MSVC)
  target_compile_definitions(antlr PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()

# Java runs ANTLR (the script parser) and jar (the StoryDesigner runtime packages)
if (OS STREQUAL "android")
  INCLUDE(FindJava)
  if (NOT Java_JAVA_EXECUTABLE)
    set(Java_JAVA_EXECUTABLE dalvikvm CACHE FILEPATH "" FORCE)
  endif()
else()
  find_package(Java COMPONENTS Runtime Development)
  if (NOT Java_JAVA_EXECUTABLE OR NOT Java_JAR_EXECUTABLE)
    message(FATAL_ERROR "The Adventure needs a Java JDK (java and jar): set JAVA_HOME, "
      "put the JDK's bin folder in PATH, or turn the Adventure off (PROJECT_ADVENTURE=OFF)")
  endif()
endif()

SET(PARSER_SRCS
${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.c
${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.h
${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.c
${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.h
)

SET_SOURCE_FILES_PROPERTIES(${PARSER_SRCS} PROPERTIES GENERATED TRUE)
# the generated C sources are compiled as C++ (only those: a header with a
# language becomes an object file of its own)
SET_SOURCE_FILES_PROPERTIES(
${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.c
${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.c
PROPERTIES
LANGUAGE CXX
)

if(${OS} STREQUAL "android")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/antlrworks_android.jar DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
set(ANTLR ${CMAKE_CURRENT_BINARY_DIR}/antlrworks_android.jar)
else()
set(ANTLR ${CMAKE_CURRENT_SOURCE_DIR}/antlrworks-1.2.3.jar)
endif()

ADD_CUSTOM_COMMAND(OUTPUT ${PARSER_SRCS}
COMMAND ${JAVA_RUNTIME} -cp ${ANTLR} org.antlr.Tool -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/pcdk.g
DEPENDS pcdk.g
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "antlr'ing grammar"
)

configure_file(AST.h AST.h COPYONLY)

SOURCE_GROUP("Resource Files" FILES AppBuilder.rc)
SOURCE_GROUP("Grammar" FILES pcdk.g)

###AdventureBase###

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
  Console.cpp
  Console.h
  Engine.cpp
  Engine.h
  EngineObjects.cpp
  EngineObjects.h
  ExecutionContext.cpp
  ExecutionContext.h
  Font.cpp
  Font.h
  Graphics.cpp
  Graphics.h
  GuiRoom.h
  Ids.h
  Inventory.cpp
  Inventory.h
  ItemObject.cpp
  ItemObject.h
  Loader.cpp
  Loader.h
  LuaChunk.cpp
  LuaChunk.h
  Menu.cpp
  Menu.h
  Particles.cpp
  Particles.h
  PostProcessing.cpp
  PostProcessing.h
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
# the shaders (post-processing effects and the quads) are compiled in as adventure/...
cge_embed_shaders(AdventureBase ${CMAKE_CURRENT_SOURCE_DIR}/shaders adventure/ registerAdventureShaders)

# the generated parser and the copied AST.h are in the binary directory
target_include_directories(AdventureBase PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(AdventureBase PUBLIC CGE::Engine antlr xml)

IF(DISABLE_EFX)
  target_compile_definitions(AdventureBase PUBLIC DISABLE_EFX)
ENDIF()

find_package(OpenAL)
if (NOT OPENAL_FOUND)
	if (OS STREQUAL "windows")
		# the StoryDesigner's runtime packages (engine2.dat) contain OpenAL32.dll
		message(FATAL_ERROR "The Adventure needs OpenAL on Windows: set OPENAL_LIBRARY and OPENAL_INCLUDE_DIR, "
			"or turn the Adventure off (PROJECT_ADVENTURE=OFF)")
	endif()
	message(WARNING "Setting option DISABLE_SOUND as OpenAL was not found")
	set(DISABLE_SOUND ON CACHE BOOL "" FORCE)
endif()

IF(NOT DISABLE_SOUND)
###Open AL###
target_include_directories(AdventureBase PUBLIC
	${OPENAL_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}/..
)
target_link_libraries(AdventureBase PUBLIC ${OPENAL_LIBRARY})

###ALUT###
ADD_SUBDIRECTORY(alut)
target_link_libraries(AdventureBase PUBLIC alut)

###FFMPEG###
SET(FFMPEG_LIBS avformat avcodec avutil swscale)
IF(NOT FFMPEG_OLD_API)
  LIST(APPEND FFMPEG_LIBS swresample)
ENDIF()
IF (OS STREQUAL windows)
  # prebuilt FFmpeg 2.x import libraries and headers
  foreach(lib ${FFMPEG_LIBS})
    add_library(${lib} STATIC IMPORTED)
    set_target_properties(${lib} PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/lib/${CPU}/${lib}.lib
      INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/include
    )
  endforeach()
ELSEIF (OS STREQUAL "android")
  foreach(lib ${FFMPEG_LIBS})
    add_library(${lib} STATIC IMPORTED)
    set_target_properties(${lib} PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/../../../extern/ffmpeg/c4droid_build/lib${lib}/lib${lib}.a
      INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/include
    )
  endforeach()
ELSE()
  # system FFmpeg
  target_link_directories(AdventureBase PUBLIC /usr/local/lib)
ENDIF()
target_link_libraries(AdventureBase PUBLIC ${FFMPEG_LIBS})

ELSE(NOT DISABLE_SOUND)
target_compile_definitions(AdventureBase PUBLIC DISABLE_SOUND)
ENDIF(NOT DISABLE_SOUND)
IF(FFMPEG_OLD_API)
target_compile_definitions(AdventureBase PUBLIC FFMPEG_OLD_API)
ENDIF(FFMPEG_OLD_API)

# the Lua module (a shared library loaded by Lua) doesn't exist on the web
if (NOT EMSCRIPTEN)
  ADD_SUBDIRECTORY(lua)
endif()

###Adventure###

IF(JNI_FRONTEND)

ADD_LIBRARY(Adventure SHARED
	${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

ADD_SUBDIRECTORY(java)

elseif(OS STREQUAL "android")

ADD_LIBRARY(Adventure SHARED
	${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)


ELSE(JNI_FRONTEND)

ADD_EXECUTABLE(Adventure
  ${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

ENDIF(JNI_FRONTEND)

TARGET_LINK_LIBRARIES(Adventure PRIVATE AdventureBase ${ADVENTURE_LINK_LIBS})

IF (NOT RENDER_EMBEDDED)
SET_TARGET_PROPERTIES(Adventure
PROPERTIES
WIN32_EXECUTABLE TRUE)
ENDIF(NOT RENDER_EMBEDDED)

# the web page Adventure.html; a game comes as a data package that the page
# loads (Adventure.html?data=demo, see test/CMakeLists.txt) to data/game.dat
cge_web_application(Adventure PRELOAD ffmpeg/bin/engine.ini@engine.ini)

ADD_SUBDIRECTORY(eax2efx)

if(NOT RENDER_EMBEDDED AND NOT EMSCRIPTEN)
# engine1.dat is the runtime the StoryDesigner unpacks: the Adventure, the
# engine library and its OpenGL backend plugin
add_custom_command(TARGET Adventure POST_BUILD
COMMAND ${Java_JAR_EXECUTABLE} cfM engine1.dat -C $<TARGET_FILE_DIR:Adventure> $<TARGET_FILE_NAME:Adventure>
  -C $<TARGET_FILE_DIR:Engine> $<TARGET_FILE_NAME:Engine>
  -C $<TARGET_FILE_DIR:EngineGL> $<TARGET_FILE_NAME:EngineGL>)
endif()

#install

cge_copy_runtime(Adventure)
INSTALL(TARGETS Adventure RUNTIME DESTINATION adventure LIBRARY	DESTINATION adventure COMPONENT adv_runtime)
cge_install_runtime(Adventure adventure COMPONENT adv_runtime)
if (NOT EMSCRIPTEN)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine1.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
endif()

if (OS STREQUAL windows)
#openal
get_filename_component(OPENAL_PATH ${OPENAL_LIBRARY} PATH)
set (OPENAL_PATH ${OPENAL_PATH}/../bin)
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
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avcodec-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avformat-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avutil-52.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swresample-0.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swscale-2.dll
)

add_custom_target(pack_engine_deps ALL DEPENDS engine2.dat)

INSTALL(FILES ${OPENAL_DLL} DESTINATION adventure)

#ffmpeg
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avcodec-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avformat-55.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/avutil-52.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swresample-0.dll
${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/${CPU}/swscale-2.dll
DESTINATION adventure)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine2.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
endif()

# the web page has it in its data
if (NOT EMSCRIPTEN)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/bin/engine.ini DESTINATION adventure)
endif()
