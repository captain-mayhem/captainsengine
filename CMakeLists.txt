cmake_minimum_required(VERSION 3.21)

project(CaptainsEngine C CXX)

# true if the engine is built on its own, false if another project (e.g. a game)
# pulls it in with add_subdirectory()
set(CGE_IS_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# target platform, derived from the (cross-)compilation target
if (ANDROID)
  set(OS "android")
elseif (WIN32)
  set(OS "windows")
elseif (UNIX)
  set(OS "linux")
else()
  message(FATAL_ERROR "Unsupported target platform ${CMAKE_SYSTEM_NAME}")
endif()
message(STATUS "Compiling for ${OS}")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPU "x64")
else()
  set(CPU "x86")
endif()
message(STATUS "CPU architecture is ${CPU}")

# Everything uses the DLL runtime (/MD, /MDd) so that memory and STL objects
# can cross the engine library boundary.
if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# C++20 for the engine and all its projects (the Engine target also requires it
# from its users)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# the engine's own projects are only built by default when the engine is the top-level project
option(PROJECT_HEROQUEST "compile heroquest projects" ${CGE_IS_TOP_LEVEL})
option(PROJECT_ADVENTURE "compile adventure projects" ${CGE_IS_TOP_LEVEL})
option(PROJECT_OSMRACER "compile osm racer projects" ${CGE_IS_TOP_LEVEL})
option(PROJECT_COMPILER "compile compiler project" OFF)
option(PROJECT_VGRENDER "compile vector graphics render projects" OFF)
set(STORY_DESIGNER_VERSION "1.1.0" CACHE STRING "Story Designer Version" FORCE)

# physics (ODE) is a separate library; only OSMRacer uses it
option(CGE_WITH_PHYSICS "build the physics library (CGE::Physics, based on ODE)" ${PROJECT_OSMRACER})
option(PHYSICS_DOUBLE_PRECISION "enable double precision physics calculations" ON)
if (PROJECT_OSMRACER AND NOT CGE_WITH_PHYSICS)
  message(FATAL_ERROR "OSMRacer needs CGE_WITH_PHYSICS")
endif()

# unit tests (googletest, run with ctest); by default only for desktop builds
# of the engine itself, since they run on the build machine
if (CGE_IS_TOP_LEVEL AND NOT ANDROID AND NOT EMSCRIPTEN)
  set(CGE_BUILD_TESTS_DEFAULT ON)
else()
  set(CGE_BUILD_TESTS_DEFAULT OFF)
endif()
option(CGE_BUILD_TESTS "build the engine's unit tests" ${CGE_BUILD_TESTS_DEFAULT})

include(FindFBX)

add_subdirectory(extern)
add_subdirectory(Engine)
if (CGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
if (PROJECT_HEROQUEST)
  add_subdirectory(editor)
  add_subdirectory(heroquest)
endif()
if (PROJECT_ADVENTURE)
  add_subdirectory(Adventure)
endif()
if (PROJECT_OSMRACER)
  add_subdirectory(OSMRacer)
endif()
if (PROJECT_COMPILER)
  add_subdirectory(Compiler)
endif()
if (PROJECT_VGRENDER)
  add_subdirectory(vgrender)
endif()
add_subdirectory(tools)

# packaging is only set up for the engine's own build; a project that includes
# the engine configures CPack itself
IF (NOT CGE_IS_TOP_LEVEL)
  RETURN()
ENDIF()

#packaging
set(CPACK_GENERATOR "NSIS")
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGE_NAME "CaptainsGameEngine")
set(CPACK_PACKAGE_VENDOR "Marco Staginski")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An open source, cross-platform game engine with focus on point&click adventures and 2D games")
set(CPACK_PACKAGE_VERSION ${STORY_DESIGNER_VERSION})

set(CPACK_COMPONENT_GROUP_EDITOR_DISPLAY_NAME "StoryDesigner")
set(CPACK_COMPONENT_GROUP_EDITOR_DESCRIPTION "The Point&Click Adventure and 2D Game Editor")
set(CPACK_COMPONENT_GROUP_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_GROUP_EXAMPLES_DESCRIPTION "Examples and Demos")

set(CPACK_COMPONENTS_ALL StoryDesignerRuntime StoryDesignerPlugins StoryDesignerEngine TestAdventure Airfight)
set(CPACK_COMPONENT_STORYDESIGNERRUNTIME_GROUP "Editor")
set(CPACK_COMPONENT_STORYDESIGNERRUNTIME_DISPLAY_NAME "StoryDesigner")
set(CPACK_COMPONENT_STORYDESIGNERRUNTIME_DESCRIPTION "The main component of the editor")
set(CPACK_COMPONENT_STORYDESIGNERRUNTIME_REQUIRED TRUE)
set(CPACK_COMPONENT_STORYDESIGNERPLUGINS_GROUP "Editor")
set(CPACK_COMPONENT_STORYDESIGNERPLUGINS_DISPLAY_NAME "StoryDesigner Plugins")
set(CPACK_COMPONENT_STORYDESIGNERPLUGINS_DESCRIPTION "Useful plugins for the editor like remote engine control, trace viewer, etc.")
set(CPACK_COMPONENT_STORYDESIGNERENGINE_GROUP "Editor")
set(CPACK_COMPONENT_STORYDESIGNERENGINE_DISPLAY_NAME "CGE Runtime")
set(CPACK_COMPONENT_STORYDESIGNERENGINE_DESCRIPTION "The game engine runtime environment for Windows")
set(CPACK_COMPONENT_TESTADVENTURE_GROUP "Examples")
set(CPACK_COMPONENT_TESTADVENTURE_DISPLAY_NAME "Demo-Adventure")
set(CPACK_COMPONENT_TESTADVENTURE_DESCRIPTION "A simple demo adventure that gives an example how editor and engine can be used")
set(CPACK_COMPONENT_AIRFIGHT_GROUP "Examples")
set(CPACK_COMPONENT_AIRFIGHT_DISPLAY_NAME "Airfight")
set(CPACK_COMPONENT_AIRFIGHT_DESCRIPTION "A simple 2D shooter with airplanes using Lua scripting")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "CaptainsEngine")

set(CPACK_NSIS_EXECUTABLES_DIRECTORY "StoryDesigner")
set(CPACK_PACKAGE_EXECUTABLES StoryDesigner StoryDesigner)
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "StoryDesigner")
include(CPack)