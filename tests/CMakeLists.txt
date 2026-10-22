# Unit tests of the engine library (googletest), run with ctest.
# They need no window and no graphics, so they also run on CI machines; the
# OpenGL tests (test_glcontext.cpp, test_gldevice.cpp) skip themselves there.

add_executable(cge_tests
	main.cpp
	test_assets.cpp
	test_drawing.cpp
	test_filesystem.cpp
	test_glcontext.cpp
	test_gldevice.cpp
	test_images.cpp
	test_imgui.cpp
	test_renderdevice.cpp
	test_services.cpp
	test_settings.cpp
	test_userdirs.cpp
	test_zip.cpp
	testenv.h
	testwindow.h
)
target_link_libraries(cge_tests PRIVATE CGE::Engine GTest::gtest)
# the test files are read from the source tree, independent of the working directory
target_compile_definitions(cge_tests PRIVATE CGE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
# a shader folder compiled in, for ShaderLibrary::addFiles
cge_embed_shaders(cge_tests ${CMAKE_CURRENT_SOURCE_DIR}/data/embedded test/ registerTestShaders)
# CGE.dll and the render backend plugins next to the executable
cge_copy_runtime(cge_tests)

include(GoogleTest)
# PRE_TEST: the tests are listed when ctest runs, after CGE.dll was copied
gtest_discover_tests(cge_tests DISCOVERY_MODE PRE_TEST)
