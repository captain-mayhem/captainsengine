-- The Salt Cellars: the second quest of Torchdelve.
-- Defeat the goblin chief Snikkerbrand (K), then climb back up the stair (A).

function intro()
  local text = "Cindermere lives on salt, and something is stealing it. Night after night the " ..
    "salt store under the market hall loses whole sacks, and the pickers talk of small grey " ..
    "hands and laughter in the old galleries. Find the thieves' chief and put an end to it. " ..
    "The Salt Guild pays thirty gold to every hero who comes back up the stair once the " ..
    "chief has fallen.";
  messageBox(text);
end

chiefDead = false;

function levelInit()
end

function eventScheduler(script)
  if script == "K" then
    return "OnInit";
  elseif script == "T" or script == "L" then
    return "OnTreasure";
  elseif script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

-- Snikkerbrand: an orc who calls himself king of the goblins
function K(sx, sy)
  setCreatureProperty(sx, sy, "name", "Snikkerbrand");
  setCreatureProperty(sx, sy, "body", 3);
  setCreatureProperty(sx, sy, "defense", 3);
  function chiefHit(tx, ty)
    if getCreatureProperty(tx, ty, "body") <= 0 then
      chiefDead = true;
      output("Snikkerbrand falls, and his goblins howl.", "Snikkerbrand has fallen.");
    else
      addEntry("hit", tx, ty, chiefHit);
    end
  end
  addEntry("hit", sx, sy, chiefHit);
end

function T(sx, sy)
  changeMoney(sx, sy, 40);
  output("Under the wet salt at the bottom of the chest lie 40 gold coins.",
    getCreatureProperty(sx, sy, "name") .. " found 40 gold.");
end

function L(sx, sy)
  changeMoney(sx, sy, 60);
  local txt = "In Snikkerbrand's chest: 60 gold and a scorched letter. 'Salt for the Grey " ..
    "Abbot, a sack each new moon, delivered to the drowned chapel. The Abbot pays in shadow.'";
  output(txt, getCreatureProperty(sx, sy, "name") .. " found 60 gold and a scorched letter.");
end

function A(sx, sy)
  if not chiefDead then
    output("The goblins' chief still rules down here.", "");
    return false;
  end
  changeMoney(sx, sy, 30);
  output("You climb back into the market hall. The Salt Guild pays you 30 gold.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Goblin");
end
