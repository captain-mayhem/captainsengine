-- The Ossuary Bridge: the fourth quest of Torchdelve.
-- Pull both levers (L, R) to open the bone gate at the bridge's end, then take the far stair (A).

function intro()
  local text = "The Ashen Court lies beyond the Ossuary Bridge, a span of fused bones over a " ..
    "chasm with no bottom. Where the bridge meets the far bank, the bones grow into a wall. " ..
    "The miners' tale says two levers on this side open it, one in a chamber to the north " ..
    "and one to the south. Open the gate, cross the bridge and find the stair down on the " ..
    "other side. What sleeps in the ossuary does not like to be woken.";
  messageBox(text);
end

leverL = false;
leverR = false;

function levelInit()
end

function eventScheduler(script)
  if script == "L" or script == "R" or script == "A" then
    return "OnWalkAt";
  elseif script == "G" then
    return "OnInit";
  else
    return "None";
  end
end

-- Gravewing, the gargoyle that keeps the far bank
function G(sx, sy)
  setCreatureProperty(sx, sy, "name", "Gravewing");
end

-- a lever wakes the dead around the hero who pulls it
function wakeTheDead(sx, sy)
  local free, tx, ty = getAdjacentFreeField(sx, sy);
  if tx ~= -1 then
    setMonster(tx, ty, "Skeleton");
  end
end

function pullLever(sx, sy)
  local name = getCreatureProperty(sx, sy, "name");
  if leverL and leverR then
    openDoor(19, 5, "RIGHT");
    openDoor(19, 6, "RIGHT");
    output("You pull the second lever. Far away, bone grinds on bone: the gate is open.",
      name .. " pulled the second lever. The bone gate is open.");
  else
    output("The lever moves with a crack of bones. Something stirs beside you.",
      name .. " pulled a lever. One more to go.");
  end
  wakeTheDead(sx, sy);
  return true;
end

function L(sx, sy)
  leverL = true;
  return pullLever(sx, sy);
end

function R(sx, sy)
  leverR = true;
  return pullLever(sx, sy);
end

function A(sx, sy)
  changeMoney(sx, sy, 40);
  output("You leave the ossuary behind and find 40 gold on the stair's last step.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Skeleton");
end
