if (NOT CMAKE_HOST_WIN32)
  return()
endif()
ADD_SUBDIRECTORY(sharpzip)
ADD_SUBDIRECTORY(StoryDesigner)

SET(STORY_DESIGNER_VERSION "${STORY_DESIGNER_VERSION}.0")
SET(ENGINE_VERSION "1.0.4.0")
SET(DEP_LIB_VERSION "1.0.4.0")

SET(ENGINE_FILE "${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.dat")
SET(DEP_LIB_FILE "${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.dat")

configure_file(configure_update.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake @ONLY)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/update.xml
  COMMAND ${CMAKE_COMMAND} -DSTORY_DESIGNER_FILE=${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/StoryDesigner/bin/${CMAKE_CFG_INTDIR}/StoryDesigner.exe 
    -P ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/update.xml.in
  COMMENT "Configuring update.xml"
  VERBATIM)

add_custom_target(configure_update ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/update.xml)
  
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/update.xml DESTINATION StoryDesignerUpdate)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.ver ${ENGINE_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine1.ver DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.ver ${DEP_LIB_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AdvEngine/Engine/engine2.ver DESTINATION StoryDesigner COMPONENT StoryDesignerEngine)
