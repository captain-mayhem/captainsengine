
call untar.cmd ode-0.11
set DIR=ode-0.11
set OUTDIR=%1\ode
md %OUTDIR%
xcopy /S /I %DIR%\include\ode %OUTDIR%\ode
copy %DIR%\ode\src\*.cpp %OUTDIR%\
copy %DIR%\ode\src\*.c %OUTDIR%\
copy %DIR%\ode\src\*.h %OUTDIR%\
xcopy /S /I %DIR%\ode\src\joints %OUTDIR%\joints
copy %DIR%\OPCODE\*.cpp %OUTDIR%\
copy %DIR%\OPCODE\*.h %OUTDIR%\
xcopy /S /I %DIR%\OPCODE\Ice %OUTDIR%\Ice
del /S /Q /F %DIR%
rd /S /Q %DIR%