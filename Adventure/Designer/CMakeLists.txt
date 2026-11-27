# The StoryDesigner in C++: the Adventure's editor on the engine's ImGui, a
# desktop program (Windows and Linux). It replaces the C# StoryDesigner in
# Editor/ step by step.
if (EMSCRIPTEN OR ANDROID)
  return()
endif()

# the project model, its files and its editing, without a window
add_library(DesignerModel STATIC
  model/bitmapfont.cpp
  model/bitmapfont.h
  model/gamefile.cpp
  model/gamefile.h
  model/images.cpp
  model/images.h
  model/languages.cpp
  model/languages.h
  model/pcdkparser.cpp
  model/pcdkparser.h
  model/project.cpp
  model/project.h
  model/projectfile.cpp
  model/projectfile.h
  model/resources.cpp
  model/resources.h
  model/roomedit.cpp
  model/roomedit.h
  model/scriptcheck.cpp
  model/scriptcheck.h
  model/stateframes.cpp
  model/stateframes.h
)
target_include_directories(DesignerModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# the game's script parser checks the scripts; FreeType renders the game's
# fonts, libjpeg writes the PNGs that a game has as JPEGs
target_link_libraries(DesignerModel PUBLIC CGE::Engine AdventureBase PRIVATE freetype JPEG::JPEG)

# the editor; the target isn't called StoryDesigner, the C# one's name
cge_add_application(Designer WIN32
  app/designer.cpp
  app/designer.h
  app/filedialog.cpp
  app/filedialog.h
  app/gamewindows.cpp
  app/imagecache.cpp
  app/imagecache.h
  app/main.cpp
  app/process.cpp
  app/process.h
  app/projectwindows.cpp
  app/resourcewindows.cpp
  app/roomeditor.cpp
  app/roomeditor.h
  app/scripteditor.cpp
  app/scripteditor.h
  app/stateframeeditor.cpp
  app/stateframeeditor.h
  app/widgets.cpp
  app/widgets.h
)
target_link_libraries(Designer PRIVATE DesignerModel)
# started from the build folder, it finds its engine.ini there
add_custom_command(TARGET Designer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/app/engine.ini $<TARGET_FILE_DIR:Designer>/engine.ini)
cge_copy_runtime(Designer)
install(TARGETS Designer RUNTIME DESTINATION designer)
cge_install_runtime(Designer designer)
install(FILES app/engine.ini DESTINATION designer)

# Create game unpacks the runtime from next to the Designer: engine1.dat (the
# Adventure and the engine's libraries, made by the Adventure's build),
# engine2.dat (the engine's fonts) and font.dat (the game's own font)
set(SD_GAME_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/StoryDesigner/StoryDesigner/Resources/font.dat)
set(SD_ENGINE_DATS ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.dat ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.dat)
if (TARGET pack_engine_deps)
  add_dependencies(Designer Adventure pack_engine_deps)
  add_custom_command(TARGET Designer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SD_GAME_FONT} ${SD_ENGINE_DATS} $<TARGET_FILE_DIR:Designer>)
  install(FILES ${SD_GAME_FONT} ${SD_ENGINE_DATS} DESTINATION designer)
endif()
# the video converter for the videos the engine can't play (Windows)
if (TARGET videoconv)
  add_dependencies(Designer videoconv)
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(SD_VIDEOCONV ${CMAKE_CURRENT_BINARY_DIR}/../Editor/videoconv-win32/out/videoconv.exe)
  else()
    set(SD_VIDEOCONV $<TARGET_FILE:videoconv>)
  endif()
  add_custom_command(TARGET Designer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SD_VIDEOCONV} $<TARGET_FILE_DIR:Designer>)
  install(PROGRAMS ${SD_VIDEOCONV} DESTINATION designer)
endif()

if (CGE_BUILD_TESTS)
  add_executable(designer_tests
    tests/test_gamefile.cpp
    tests/test_projectfile.cpp
    tests/test_roomedit.cpp
    tests/test_scripts.cpp
    tests/test_stateframes.cpp
  )
  target_link_libraries(designer_tests PRIVATE DesignerModel GTest::gtest GTest::gtest_main)
  # the test games and the files the C# StoryDesigner wrote from them
  target_compile_definitions(designer_tests PRIVATE
    SD_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
    SD_TEST_GAMES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../AdvEngine/test")
  cge_copy_runtime(designer_tests)
  include(GoogleTest)
  gtest_discover_tests(designer_tests DISCOVERY_MODE PRE_TEST PROPERTIES TIMEOUT 120)
endif()
