# The StoryDesigner in C++: the Adventure's editor on the engine's ImGui, a
# desktop program (Windows and Linux). It replaces the C# StoryDesigner in
# Editor/ step by step.
if (EMSCRIPTEN OR ANDROID)
  return()
endif()

# the project model, its files and its editing, without a window
add_library(DesignerModel STATIC
  model/project.cpp
  model/project.h
  model/projectfile.cpp
  model/projectfile.h
)
target_include_directories(DesignerModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DesignerModel PUBLIC CGE::Engine)

# the editor; the target isn't called StoryDesigner, the C# one's name
cge_add_application(Designer WIN32
  app/designer.cpp
  app/designer.h
  app/filedialog.cpp
  app/filedialog.h
  app/main.cpp
)
target_link_libraries(Designer PRIVATE DesignerModel)
# started from the build folder, it finds its engine.ini there
add_custom_command(TARGET Designer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/app/engine.ini $<TARGET_FILE_DIR:Designer>/engine.ini)
cge_copy_runtime(Designer)
install(TARGETS Designer RUNTIME DESTINATION designer)
cge_install_runtime(Designer designer)
install(FILES app/engine.ini DESTINATION designer)

if (CGE_BUILD_TESTS)
  add_executable(designer_tests
    tests/test_projectfile.cpp
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
