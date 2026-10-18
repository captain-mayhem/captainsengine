if (NOT CMAKE_HOST_WIN32)
  return()
endif()

# The C# projects are built by MSBuild in custom targets. As projects in the
# generated solution they only built there: cmake --build builds ALL_BUILD,
# which passes its platform (x64) on to them instead of "Any CPU".
if (CMAKE_VS_MSBUILD_COMMAND)
  set(CGE_MSBUILD ${CMAKE_VS_MSBUILD_COMMAND})
else()
  find_program(CGE_MSBUILD MSBuild)
endif()
if (NOT CGE_MSBUILD)
  message(FATAL_ERROR "The StoryDesigner needs MSBuild: use the Visual Studio generator or a developer prompt, "
    "or turn the Adventure off (PROJECT_ADVENTURE=OFF)")
endif()
# the C# projects only have Debug and Release
set(CGE_CSHARP_CONFIG $<IF:$<CONFIG:Debug>,Debug,Release>)

function(cge_add_csharp_project target csproj)
  add_custom_target(${target} ALL
    COMMAND ${CGE_MSBUILD} ${csproj} -nologo -v:m -nodeReuse:false
      -p:Configuration=${CGE_CSHARP_CONFIG} -p:Platform=AnyCPU
    SOURCES ${csproj}
    COMMENT "Building ${target} (C#)"
    VERBATIM)
endfunction()

ADD_SUBDIRECTORY(sharpzip)
ADD_SUBDIRECTORY(StoryDesigner)

SET(STORY_DESIGNER_VERSION_ORIG "${STORY_DESIGNER_VERSION}")
SET(STORY_DESIGNER_VERSION "${STORY_DESIGNER_VERSION}.0")
SET(ENGINE_VERSION "1.1.0.0")
SET(DEP_LIB_VERSION "1.0.3.0")

SET(ENGINE_FILE "${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.dat")
SET(DEP_LIB_FILE "${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.dat")

configure_file(configure_update.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake @ONLY)

if (CPU STREQUAL "x86")
set(INST_ARCH "win32")
else()
set(INST_ARCH "win64")
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/update.xml
  COMMAND ${CMAKE_COMMAND} -DSTORY_DESIGNER_FILE=${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/StoryDesigner/bin/${CGE_CSHARP_CONFIG}/StoryDesigner.exe
-DINSTALLER_FILE=${CMAKE_INSTALL_PREFIX}/CaptainsGameEngine-${STORY_DESIGNER_VERSION_ORIG}-${INST_ARCH}.exe
    -P ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/update.xml.in
  COMMENT "Configuring update.xml"
  VERBATIM)

add_custom_target(configure_update ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/update.xml)
# update.xml hashes StoryDesigner.exe, engine1.dat (the Adventure's post-build
# step creates it) and, on Windows, engine2.dat
add_dependencies(configure_update Adventure StoryDesigner pack_engine_deps)
  
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/update.xml DESTINATION StoryDesignerUpdate)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.ver ${ENGINE_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.ver DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.ver ${DEP_LIB_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.ver DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
