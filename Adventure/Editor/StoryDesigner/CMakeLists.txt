include_external_msproject(
    StoryDesigner ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/StoryDesigner.csproj
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
    PLATFORM "Any CPU")
    
ADD_CUSTOM_TARGET(BuildStoryDesigner ALL DEPENDS StoryDesigner)
    
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/StoryDesigner.exe DESTINATION StoryDesigner CONFIGURATIONS Debug)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/de DESTINATION StoryDesigner CONFIGURATIONS Debug)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/de-DE DESTINATION StoryDesigner CONFIGURATIONS Debug)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/StoryDesigner.exe DESTINATION StoryDesigner CONFIGURATIONS Release)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/de DESTINATION StoryDesigner CONFIGURATIONS Release)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/de-DE DESTINATION StoryDesigner CONFIGURATIONS Release)

include_external_msproject(
    RemoteControl ${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteControl.csproj
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
    PLATFORM "Any CPU")

ADD_CUSTOM_TARGET(BuildRemoteControl ALL DEPENDS RemoteControl)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/RemoteControl.dll DESTINATION StoryDesigner CONFIGURATIONS Debug)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/bin/Release/RemoteControl.dll DESTINATION StoryDesigner CONFIGURATIONS Release)