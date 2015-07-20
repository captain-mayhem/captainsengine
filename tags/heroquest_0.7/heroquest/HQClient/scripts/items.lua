function item(sx, sy, tx, ty, idx)

  -- healing potion
  if idx == 0 then
    local points = getCreatureProperty(sx, sy, "body");
    local maxpoints = getCreatureProperty(sx, sy, "maxbody");
    -- heal 4 points
    points = points+4;
    -- don't go over the maximum
    if points > maxpoints then
      points = maxpoints;
    end
    setCreatureProperty(sx, sy, "body", points);
    return true;
  
  -- heroic brew
  elseif idx == 1 then
    allowAnotherAction("attack");
    return true;

  -- potion of defence
  elseif idx == 2 then
    local defend = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", defend+2);

		function restoreDef(tx, ty)
			local defend = getCreatureProperty(tx, ty, "defence");
    	setCreatureProperty(tx, ty, "defence", defend-2);
		end
    
    addEntry("attack", sx, sy, restoreDef);
    return true;

  -- potion of strength
  elseif idx == 3 then
    local attack = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", attack+2);

		function restoreAtt(tx, ty)
			local attack = getCreatureProperty(tx, ty, "attack");
    	setCreatureProperty(tx, ty, "attack", attack-2);
		end
    
		addEntry("endTurn", sx, sy, restoreAtt);	
    return true;

  -- purple liquid
  elseif idx == 5 then
    setCreatureProperty(sx, sy, "sleeping", true);
    output("You turn into a statue for 5 turns", "");
    local count = 0;
    function purpleLiqWakeUp(tx, ty)
      count = count+1;
      if count == 5 then
        setCreatureProperty(tx, ty, "sleeping", false);
      else
        addEntry("startTurn", tx, ty, purpleLiqWakeUp)
      end
    end
    forceEndTurn();
    addEntry("startTurn", sx, sy, purpleLiqWakeUp);
    return true;

  --wand of magic
  elseif idx == 6 then
    local charId = getCreatureProperty(sx, sy, "id");
    --elf or wizard
    if charId == 2 or charId == 3 then
      allowAnotherAction("spell");
      return true;
    else
      output("You cannot use this item.","");
      return false;
    end
    
  end

  return false;
end
