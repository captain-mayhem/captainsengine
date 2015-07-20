
call untar.cmd zlib-1.2.3
set DIR=zlib-1.2.3
set OUTDIR=%1\zlib
md %OUTDIR%
copy %DIR%\*.c %OUTDIR%\
copy %DIR%\*.h %OUTDIR%\
copy %DIR%\contrib\minizip\*.c %OUTDIR%\
copy %DIR%\contrib\minizip\*.h %OUTDIR%\
del /S /Q /F %DIR%
rd /S /Q %DIR%