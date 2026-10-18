cge_add_csharp_project(CSharpZip ${CMAKE_CURRENT_SOURCE_DIR}/SrcSamples/src/ICSharpCode.SharpZLib.csproj)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/SrcSamples/bin/ICSharpCode.SharpZipLib.dll DESTINATION StoryDesigner COMPONENT StoryDesignerRuntime)
