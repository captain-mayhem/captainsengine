###OPTIONS###
# The sound goes through the engine's audio module (Sound.cpp, SoundPlayer.cpp)
# on every platform.

# the frontend (window and main loop): an engine application that draws
# through the engine's render device, without own GL calls, on every platform
# (on the web in the browser, on Android in the engine's CgeActivity)
SET(ADVENTURE_SRCS CgeWindow.cpp CgeWindow.h)

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

# Java runs ANTLR (the script parser) and jar (the StoryDesigner runtime
# packages), on the build machine for every platform (for Android the JDK
# that Gradle runs on)
find_package(Java COMPONENTS Runtime Development)
if (NOT Java_JAVA_EXECUTABLE OR NOT Java_JAR_EXECUTABLE)
  message(FATAL_ERROR "The Adventure needs a Java JDK (java and jar): set JAVA_HOME, "
    "put the JDK's bin folder in PATH, or turn the Adventure off (PROJECT_ADVENTURE=OFF)")
endif()

SET(PARSER_SRCS
${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.cpp
${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.h
${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.cpp
${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.h
)

SET_SOURCE_FILES_PROPERTIES(${PARSER_SRCS} PROPERTIES GENERATED TRUE)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # ANTLR's generated code has "if ((a == b))"
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/pcdkParser.cpp ${CMAKE_CURRENT_BINARY_DIR}/pcdkLexer.cpp
    PROPERTIES COMPILE_OPTIONS -Wno-parentheses-equality)
endif()

set(ANTLR ${CMAKE_CURRENT_SOURCE_DIR}/antlrworks-1.2.3.jar)

# ANTLR writes C sources, which are compiled as C++: renamed to .cpp, since
# clang deprecates C files in C++ mode
ADD_CUSTOM_COMMAND(OUTPUT ${PARSER_SRCS}
COMMAND ${Java_JAVA_EXECUTABLE} -cp ${ANTLR} org.antlr.Tool -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/pcdk.g
COMMAND ${CMAKE_COMMAND} -E rename pcdkParser.c pcdkParser.cpp
COMMAND ${CMAKE_COMMAND} -E rename pcdkLexer.c pcdkLexer.cpp
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
  StreamVideoPlayer.cpp
  StreamVideoPlayer.h
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

# the Lua module, a shared library that a stand-alone Lua interpreter loads to
# drive the engine (require "cge"); nothing in a web page or an APK loads it
if (NOT EMSCRIPTEN AND NOT ANDROID)
  ADD_SUBDIRECTORY(lua)
endif()

###Adventure###

# an executable, or on Android the library libAdventure.so that CgeActivity
# loads (the Gradle project in Adventure/android)
cge_add_application(Adventure WIN32
  ${ADVENTURE_SRCS}
  CmdReceiver.h
  CmdReceiver.cpp
)

TARGET_LINK_LIBRARIES(Adventure PRIVATE AdventureBase)

# the web page Adventure.html; a game comes as a data package that the page
# loads (Adventure.html?data=demo, see test/CMakeLists.txt) to data/game.dat
cge_web_application(Adventure PRELOAD engine.ini@engine.ini)

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
if (NOT RENDER_EMBEDDED AND NOT EMSCRIPTEN)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine1.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
endif()

# engine2.dat is the StoryDesigner's package of the engine's other files (it
# held the OpenAL and FFmpeg libraries): the engine's fonts in fonts/
if (OS STREQUAL windows)
file(GLOB CGE_FONT_FILES CONFIGURE_DEPENDS ${CGE_FONTS_DIR}/*)
add_custom_command(OUTPUT engine2.dat
COMMAND ${Java_JAR_EXECUTABLE} cfM engine2.dat -C ${CGE_FONTS_DIR}/.. fonts
DEPENDS ${CGE_FONT_FILES}
)
add_custom_target(pack_engine_deps ALL DEPENDS engine2.dat)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/engine2.dat DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
endif()

# the web page has it in its data
if (NOT EMSCRIPTEN)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/engine.ini DESTINATION adventure)
endif()
