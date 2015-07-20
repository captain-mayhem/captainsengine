ScriptName = "CGE_Export"

CGE_Export = {}

function CGE_Export:Name()
  return "CGE Export"
end

function CGE_Export:Version()
  return "1.0"
end

function CGE_Export:Description()
  return MOHO.Localize("/Scripts/Menu/Export/Description=CGE Export Script")
end

function CGE_Export:Creator()
  return "Marco Staginski"
end

function CGE_Export:UILabel()
  return MOHO.Localize("/Scripts/Menu/Export/CGEExport=CGE Export")
end

function CGE_Export:Run(moho)
  dofile("C:\\AnimeStudio\\Anime Studio Pro\\scripts\\menu\\Export\\cge2.lua")
  bla(moho)
end