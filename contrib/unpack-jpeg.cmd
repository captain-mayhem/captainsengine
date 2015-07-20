
call untar.cmd jpeg
set DIR=jpeg-6b
set OUTDIR=%1\jpeg
md %OUTDIR%
copy %DIR%\*.c %OUTDIR%\
copy %DIR%\*.h %OUTDIR%\
del /S /Q /F %DIR%
rd /S /Q %DIR%