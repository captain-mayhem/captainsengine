function default_wandering_monster(sx, sy, name)
  local adjacent;
  local tx;
  local ty;
  adjacent, tx, ty = getAdjacentFreeField(sx, sy);
  if tx == -1 and ty == -1 then
    output("Something moves in the dark nearby, but it passes you by.", "");
    return true;
  end
  setMonster(tx,ty,name);
  local tofinder = "The noise of your search has drawn something out of the dark"
  local toall = getCreatureProperty(sx,sy,"name") .. " was found by a wandering monster.";
  if adjacent then
    tofinder = tofinder .. ", and it strikes at once!";
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
  output("The floor gives way and you drop into a pit.", "");
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
  output("A click under your foot, and spears shoot out of the floor.", "");
  makeOverlayActive(sx, sy, false);
  releaseBackPointer(sx, sy);
  setMoves(0);
  attack(1, "normal", 0, "normal", sx, sy, sx, sy);
end

--rock trap
function c(sx, sy)
  output("The ceiling groans, and rocks come crashing down on you.", "");
  makeOverlayActive(sx, sy, true);
  --may move away
  setMoves(1);
  attack(3, "normal", 0, "normal", sx, sy, sx, sy);
  addModel(sx, sy, 2);
  return false;
end

--needle trap
function d(sx, sy)
  output("A needle springs out of the lock and pricks your finger. You feel sick.", "");
  attack(1, "damage", 0, "normal", sx, sy, sx, sy);
end
