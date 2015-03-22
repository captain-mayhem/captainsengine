ADD_SUBDIRECTORY(sharpzip)
ADD_SUBDIRECTORY(StoryDesigner)

SET(STORY_DESIGNER_VERSION "1.0.2.0")
SET(ENGINE_VERSION "1.0.2.0")
SET(DEP_LIB_VERSION "1.0.2.0")

#configure_file(StoryDesigner/StoryDesigner/Update/update.xml ${CMAKE_CURRENT_BINARY_DIR}/StoryDesigner/update.xml)
#set(FOO "Some text to show up in json_file.json")
#set(BAR "More text")
configure_file(configure_update.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake @ONLY)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/update.xml
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/configure_update.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/update.xml.in
  COMMENT "Configuring update.xml"
  VERBATIM)

add_custom_target(configure_update ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/update.xml)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AppBuilder/Appbuilder/engine1.ver ${ENGINE_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AppBuilder/Appbuilder/engine1.ver DESTINATION StoryDesigner)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/../AppBuilder/Appbuilder/engine2.ver ${DEP_LIB_VERSION})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/../AppBuilder/Appbuilder/engine2.ver DESTINATION StoryDesigner)
