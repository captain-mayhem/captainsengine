
call untar.cmd lua-5.0.2
set DIR=lua-5.0.2
set OUTDIR=%1\lua
md %OUTDIR%
copy %DIR%\src\*.c %OUTDIR%\
copy %DIR%\src\lib\*.c %OUTDIR%\
copy %DIR%\src\*.h %OUTDIR%\
copy %DIR%\include\*.h %OUTDIR%\
del /S /Q /F %DIR%
rd /S /Q %DIR%