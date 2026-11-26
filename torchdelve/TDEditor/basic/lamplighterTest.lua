-- The Lamplighter's Test: the first quest of Torchdelve.
-- Light the three beacons (P, Q, R: walk onto them), then leave by the stair (A).

function intro()
  local text = "Beneath the Lantern House of Cindermere lie the old cellars where the " ..
    "Lantern Wardens train their recruits. Warden-Captain Odila Fenn has filled them with " ..
    "creatures caught in the mines. Your test: light the three cold beacons in the west, " ..
    "the east and the south-west chambers, then find the stair down to the next cellar. " ..
    "Nobody leaves before all three beacons burn. Odila pays ten gold to whoever lights a " ..
    "beacon.";
  messageBox(text);
end

beacons = 0;

function levelInit()
end

function eventScheduler(script)
  if script == "P" or script == "Q" or script == "R" or script == "A" then
    return "OnWalkAt";
  elseif script == "T" then
    return "OnTreasure";
  else
    return "None";
  end
end

function lightBeacon(sx, sy)
  beacons = beacons + 1;
  changeMoney(sx, sy, 10);
  local name = getCreatureProperty(sx, sy, "name");
  if beacons < 3 then
    output("You light the beacon and earn ten gold. " .. (3 - beacons) .. " still to go.",
      name .. " lit a beacon.");
  else
    output("You light the last beacon. The cellars glow; the stair is open.",
      name .. " lit the last beacon. The stair is open.");
  end
  -- true: the beacon burns, the field's script is done
  return true;
end

function P(sx, sy)
  return lightBeacon(sx, sy);
end

function Q(sx, sy)
  return lightBeacon(sx, sy);
end

function R(sx, sy)
  return lightBeacon(sx, sy);
end

function T(sx, sy)
  changeMoney(sx, sy, 25);
  addItem(sx, sy, "hearthbalm");
  output("Among lamp oil and wicks you find a purse of 25 gold and a Hearthbalm.",
    getCreatureProperty(sx, sy, "name") .. " found 25 gold and a potion.");
end

function A(sx, sy)
  if beacons < 3 then
    output("The stair is barred until all three beacons burn.", "");
    return false;
  end
  output("You pass the test.", "Passed the test");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Goblin");
end
