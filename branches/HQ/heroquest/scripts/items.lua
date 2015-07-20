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
    allowAnotherAction();
    return true;

  -- potion of defence
  elseif idx == 2 then
    local defend = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", defend+2);

		function restoreDef(tx, ty)
			local defend = getCreatureProperty(tx, ty, "defence");
    	setCreatureProperty(tx, ty, "defence", defend-2);
		end
    
    addEntry("attack", tx, ty, restoreDef);
    return true;

  -- potion of strength
  elseif idx == 3 then
    local attack = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", attack+2);

		function restoreAtt(tx, ty)
			local attack = getCreatureProperty(tx, ty, "attack");
    	setCreatureProperty(tx, ty, "attack", attack-2);
		end
    
		addEntry("endTurn", tx, ty, restoreAtt);	
    return true;

  end

  return false;
end
