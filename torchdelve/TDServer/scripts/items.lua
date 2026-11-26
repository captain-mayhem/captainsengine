-- The items of data/items.dat, by their id. Returns true if the item was used up.

RANGER = 2;

function item(sx, sy, tx, ty, idx)

  -- hearthbalm: up to 4 health back
  if idx == 0 then
    local points = getCreatureProperty(sx, sy, "body") + 4;
    local maxpoints = getCreatureProperty(sx, sy, "maxbody");
    if points > maxpoints then
      points = maxpoints;
    end
    setCreatureProperty(sx, sy, "body", points);
    return true;

  -- battle ale: one more attack
  elseif idx == 1 then
    allowAnotherAction("attack");
    return true;

  -- iron tonic: 2 more defence dice until the next attack on the drinker
  elseif idx == 2 then
    local defend = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", defend+2);

    function restoreIronTonic(tx, ty)
      local defend = getCreatureProperty(tx, ty, "defence");
      setCreatureProperty(tx, ty, "defence", defend-2);
    end
    addEntry("attack", sx, sy, restoreIronTonic);
    return true;

  -- fire tonic: 2 more attack dice until the end of the turn
  elseif idx == 3 then
    local attack = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", attack+2);

    function restoreFireTonic(tx, ty)
      local attack = getCreatureProperty(tx, ty, "attack");
      setCreatureProperty(tx, ty, "attack", attack-2);
    end
    addEntry("endTurn", sx, sy, restoreFireTonic);
    return true;

  -- murky vial: nobody knows what it does, the die decides
  elseif idx == 5 then
    local die = dice("normal", 1);
    if die <= 2 then
      -- stiff as stone for 3 turns
      setCreatureProperty(sx, sy, "sleeping", true);
      output("Your limbs turn heavy as stone. You cannot move for 3 turns.", "");
      local count = 0;
      function murkyVialWakeUp(tx, ty)
        count = count+1;
        if count == 3 then
          setCreatureProperty(tx, ty, "sleeping", false);
        else
          addEntry("startTurn", tx, ty, murkyVialWakeUp);
        end
      end
      forceEndTurn();
      addEntry("startTurn", sx, sy, murkyVialWakeUp);
    elseif die <= 4 then
      output("It tastes of pond water. Nothing happens.", "");
    else
      local points = getCreatureProperty(sx, sy, "body") + 2;
      local maxpoints = getCreatureProperty(sx, sy, "maxbody");
      if points > maxpoints then
        points = maxpoints;
      end
      setCreatureProperty(sx, sy, "body", points);
      output("A pleasant warmth spreads through you. You get back 2 health.", "");
    end
    return true;

  -- twin wand: a second spell this turn, for the Ranger and the Mage
  elseif idx == 6 then
    local heroId = getCreatureProperty(sx, sy, "id");
    if heroId == RANGER or heroId == MAGE then
      allowAnotherAction("spell");
      return true;
    else
      output("Only a Ranger or a Mage can use the Twin Wand.", "");
      return false;
    end

  end

  return false;
end
