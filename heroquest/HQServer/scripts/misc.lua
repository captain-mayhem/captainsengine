function default_wandering_monster(sx, sy, name)   
  local adjacent;
  local tx;
  local ty;
  adjacent, tx, ty = getAdjacentFreeField(sx, sy);
  if tx == -1 and ty == -1 then
    output("You are lucky, becuase a wandering monster was near you, but it did not see you.", "");
    return true;
  end
  setMonster(tx,ty,name);
  local tofinder = "As you are searching, a monster stalks you"
  local toall = getCreatureProperty(sx,sy,"name") .. " was discovered by a wandering monster.";
  if adjacent then
    tofinder = tofinder .. " and attacks!";
    local attac = getCreatureProperty(tx, ty, "attack");
    local defend = getCreatureProperty(sx, sy, "defence");
    attack(attac, "normal", defend, "normal", sx, sy, sx, sy);
  else
    tofinder = tofinder .. ".";
  end
  output(tofinder, toall);
end

--schedules trap events
function trapScheduler(event)
  if event == "a" or event == "b" or event == "c" then
    return "OnWalkAt";
  elseif event == "d" then
    return "OnTreasure";
  else
    return "None";
  end
end

--pit trap
function a(sx, sy)
  output("You fall into a pit trap", "");
  makeOverlayActive(sx, sy, true);
  exchangeModel(sx, sy, 0, 1);
  moveCamera(0, -3.7, 0);
  --make creature weaker when being in the pit
  local atta = getCreatureProperty(sx, sy, "attack");
  local def = getCreatureProperty(sx, sy, "defence");
  setCreatureProperty(sx, sy, "attack", atta-1);
  setCreatureProperty(sx, sy, "defence", def-1);
  --stop it
  setMoves(0);
  --restore original when moving out
  function restorePitValues(sx, sy)
    local atta = getCreatureProperty(sx, sy, "attack");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "attack", atta+1);
    setCreatureProperty(sx, sy, "defence", def+1);
    moveCamera(0, 3.7, 0);
  end
  addEntry("move", sx, sy, restorePitValues);
  --hero may stay here
  releaseBackPointer(sx, sy);
  attack(1, "damage", 0, "normal", sx, sy, sx, sy);
  return false;
end

--spear trap
function b(sx, sy)
  output("You are hit by a spear trap", "");
  makeOverlayActive(sx, sy, false);
  releaseBackPointer(sx, sy);
  setMoves(0);
  attack(1, "normal", 0, "normal", sx, sy, sx, sy);
end

--rock trap
function c(sx, sy)
  output("You are hit by a falling rock trap", "");
  makeOverlayActive(sx, sy, true);
  --may move away
  setMoves(1);
  attack(3, "normal", 0, "normal", sx, sy, sx, sy);
  addModel(sx, sy, 2);
  return false;
end

--poison needle
function d(sx, sy)
  output("You sprung a poison needle", "");
  attack(1, "damage", 0, "normal", sx, sy, sx, sy);
end
