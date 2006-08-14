obj2hmb -k ../../../heroquest/HQClient/models/world/ground.obj
move ..\..\..\heroquest\HQClient\models\world\ground.hmb ..\..\..\heroquest\HQClient\models\
obj2hmb -k ../../../heroquest/HQClient/models/world/pit2.obj
move ..\..\..\heroquest\HQClient\models\world\pit2.hmb ..\..\..\heroquest\HQClient\models\pit.hmb
obj2hmb -f ../../../heroquest/HQClient/models/world/rocktrap.obj
move ..\..\..\heroquest\HQClient\models\world\rocktrap.hmb ..\..\..\heroquest\HQClient\models\
@set /P %path=Done
