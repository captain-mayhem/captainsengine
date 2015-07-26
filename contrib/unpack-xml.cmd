
call untar.cmd tinyxml_2_5_3
set DIR=tinyxml
set OUTDIR=%1\libxml
md %OUTDIR%
copy %DIR%\*.cpp %OUTDIR%\
copy %DIR%\*.h %OUTDIR%\
del /S /Q /F %DIR%
rd /S /Q %DIR%