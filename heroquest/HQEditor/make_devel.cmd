cd ..\HQClient
..\..\..\..\build_win\heroquest\HQEditor\Debug\HQEditor.exe ..\HQEditor\basic\maze.map ..\HQEditor\basic\trial.map
copy /Y ..\HQEditor\basic\*.hqm ..\HQClient\levels\basic\
copy /Y ..\HQEditor\basic\*.hsf ..\HQClient\levels\basic\
copy /Y ..\HQEditor\basic\*.hqm ..\HQServer\levels\basic\
copy /Y ..\HQEditor\basic\*.hsf ..\HQServer\levels\basic\
pause
REM HQEditor basic\trial.map
REM HQEditor basic\rescueSirRagnar.map
REM HQEditor basic\lairOrcWarlord.map
REM HQEditor basic\princeMagnusGold.map
REM HQEditor basic\melarsMaze.map
REM HQEditor basic\LegacyOrcWarlord.map
REM HQEditor basic\lostWizard.map
REM HQEditor basic\fireMage.map
REM HQEditor basic\raceAgainstTime.map
REM HQEditor basic\castleMystery.map
REM HQEditor basic\bastionChaos.map
REM HQEditor basic\barakTor.map
REM HQEditor basic\spiritBlade.map
REM HQEditor basic\returnBarakTor.map