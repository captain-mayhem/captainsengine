include_external_msproject(
    CSharpZip ${CMAKE_CURRENT_SOURCE_DIR}/SrcSamples/src/ICSharpCode.SharpZLib.csproj
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
    PLATFORM "Any CPU")
    
ADD_CUSTOM_TARGET(BuildCSharpZip ALL DEPENDS CSharpZip)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/SrcSamples/bin/ICSharpCode.SharpZipLib.dll DESTINATION StoryDesigner COMPONENT StoryDesignerLibs)
