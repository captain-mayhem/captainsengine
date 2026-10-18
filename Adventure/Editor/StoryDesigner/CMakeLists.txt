cge_add_csharp_project(StoryDesigner ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/StoryDesigner.csproj)
# uses the SharpZipLib dll
add_dependencies(StoryDesigner CSharpZip)
    
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/StoryDesigner.exe DESTINATION StoryDesigner CONFIGURATIONS Debug COMPONENT StoryDesignerRuntime)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/de DESTINATION StoryDesigner CONFIGURATIONS Debug COMPONENT StoryDesignerRuntime)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/de-DE DESTINATION StoryDesigner CONFIGURATIONS Debug COMPONENT StoryDesignerRuntime)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/obj/Debug/Interop.WMPLib.dll DESTINATION StoryDesigner CONFIGURATIONS Debug COMPONENT StoryDesignerRuntime)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/StoryDesigner.exe DESTINATION StoryDesigner CONFIGURATIONS Release COMPONENT StoryDesignerRuntime)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/de DESTINATION StoryDesigner CONFIGURATIONS Release COMPONENT StoryDesignerRuntime)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Release/de-DE DESTINATION StoryDesigner CONFIGURATIONS Release COMPONENT StoryDesignerRuntime)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/obj/Release/Interop.WMPLib.dll DESTINATION StoryDesigner CONFIGURATIONS Release COMPONENT StoryDesignerRuntime)

cge_add_csharp_project(RemoteControl ${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteControl.csproj)
# references the StoryDesigner project, which must not be built twice at the same time
add_dependencies(RemoteControl StoryDesigner)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/StoryDesigner/bin/Debug/RemoteControl.dll DESTINATION StoryDesigner CONFIGURATIONS Debug COMPONENT StoryDesignerPlugins)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/bin/Release/RemoteControl.dll DESTINATION StoryDesigner CONFIGURATIONS Release COMPONENT StoryDesignerPlugins)