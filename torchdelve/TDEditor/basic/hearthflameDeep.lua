-- The Hearthflame Deep: the eighth and last quest of Torchdelve.
-- Walk onto a hearth (H) to set the ember-heart back: that breaks the Shade Regent's (R)
-- ward. Defeat him, then leave by the stair (A).

function intro()
  local text = "Since the ember-heart was stolen, the Hearthflame at the bottom of the Deep " ..
    "has gone out, and Umbrath's shadow has climbed up the shafts. Its steward, the Shade " ..
    "Regent, sits in the hall north of the hearth, and while the hearth is cold, no blade " ..
    "finds him easily. Carry the ember-heart to one of the two hearths around the great " ..
    "chimney, rekindle the flame, then defeat the Shade Regent and come back up the stair. " ..
    "This is the last descent.";
  messageBox(text);
end

hearthLit = false;
regentDead = false;
regentX = -1;
regentY = -1;

function levelInit()
end

function eventScheduler(script)
  if script == "R" then
    return "OnInit";
  elseif script == "T" then
    return "OnTreasure";
  elseif script == "H" or script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

-- the Shade Regent: warded while the hearth is cold (three more defence dice)
function R(sx, sy)
  setCreatureProperty(sx, sy, "name", "Shade_Regent");
  setCreatureProperty(sx, sy, "body", 5);
  setCreatureProperty(sx, sy, "defense", 9);
  regentX = sx;
  regentY = sy;
  function regentMoved(tx, ty)
    regentX = tx;
    regentY = ty;
    addEntry("move", tx, ty, regentMoved);
  end
  addEntry("move", sx, sy, regentMoved);
  function regentHit(tx, ty)
    if getCreatureProperty(tx, ty, "body") <= 0 then
      regentDead = true;
      output("The Shade Regent comes apart like smoke in the firelight.",
        "The Shade Regent is destroyed.");
    else
      addEntry("hit", tx, ty, regentHit);
    end
  end
  addEntry("hit", sx, sy, regentHit);
end

function H(sx, sy)
  if hearthLit then
    return true;
  end
  hearthLit = true;
  setCreatureProperty(sx, sy, "body", getCreatureProperty(sx, sy, "maxbody"));
  if not regentDead and isCreatureAt(regentX, regentY) and
      getCreatureProperty(regentX, regentY, "name") == "Shade_Regent" then
    setCreatureProperty(regentX, regentY, "defense", 6);
  end
  output("You set the ember-heart into the hearth. The Hearthflame roars up the chimney, " ..
    "your wounds close in its warmth, and the Shade Regent's ward breaks.",
    getCreatureProperty(sx, sy, "name") .. " rekindled the Hearthflame. The Regent's ward is broken.");
  return true;
end

function T(sx, sy)
  addItem(sx, sy, "hearthbalm");
  addItem(sx, sy, "battle_ale");
  output("In a Warden's old chest: a Hearthbalm and a Battle Ale.",
    getCreatureProperty(sx, sy, "name") .. " found two potions.");
end

function A(sx, sy)
  if not hearthLit then
    output("The Hearthflame must burn again before you leave.", "");
    return false;
  end
  if not regentDead then
    output("The Shade Regent still rules the Deep.", "");
    return false;
  end
  changeMoney(sx, sy, 150);
  output("Cindermere's bells ring for you. The Wardens pay 150 gold to each hero.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Mummy");
end
