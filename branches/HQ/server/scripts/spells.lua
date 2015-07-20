function spell(sx, sy, tx, ty, idx)


  -- healing spell (earth) and water of healing spell (water)
  if idx == 0 or idx == 12 then
  	if not isVisible(sx, sy, tx, ty) then
  	  return false;
  	end
    local points = getCreatureProperty(tx, ty, "body");
    local maxpoints = getCreatureProperty(tx, ty, "maxbody");
    -- heal 4 points
    points = points+4;
    -- don't go over the maximum
    if points > maxpoints then
      points = maxpoints;
    end
    setCreatureProperty(tx, ty, "body", points);
    return true;

  --rock skin spell (earth)
  elseif idx == 1 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    local defence = getCreatureProperty(tx, ty, "defence");
    setCreatureProperty(tx, ty, "defence", defence+2);

    function restoreDefence(tx,ty)
      local def = getCreatureProperty(tx, ty, "defence");
      setCreatureProperty(tx, ty, "defence", def-2);
    end

    addEntry("hit", tx, ty, restoreDefence);
    return true;
    
  -- pass through rock spell (earth)
  elseif idx == 2 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    --spoken to himself
    if sx == tx and sy == ty then
      enableWallCollision(false);
    else
      --disable only when it's his turn
      function disableWallCollision(tx,ty)
        enableWallCollision(false);
      end
      addEntry("startTurn", tx, ty, disableWallCollision);
    end
      
    function restoreWallCollision(tx,ty)
      enableWallCollision(true);
    end
    addEntry("endTurn", tx, ty, restoreWallCollision);
    
    return true;

	-- fire of wrath (fire)
	elseif idx == 3 then
		attack(1, "damage", 1, "normal", sx, sy, tx, ty);
		return true;
  
	-- ball of flame (fire)
	elseif idx == 4 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
		attack(2, "damage", 2, "normal", sx, sy, tx, ty);
		return true;
  
	-- courage (fire)
	elseif idx == 5 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
		if isMonsterVisible(tx, ty) then
    	local attack = getCreatureProperty(tx, ty, "attack");
    	setCreatureProperty(tx, ty, "attack", attack+2);

			function restoreAttack(tx, ty)
    		if isMonsterVisible(tx, ty) then
					addEntry("move", tx, ty, restoreAttack);	
				else
					local attack = getCreatureProperty(tx, ty, "attack");
    			setCreatureProperty(tx, ty, "attack", attack-2);
				end
			end
			addEntry("move", tx, ty, restoreAttack);	
		end
		
		return true;
	
	-- genie1 attack
	elseif idx == 6 then
		local defence = getCreatureProperty(tx, ty, "defence");
		attack(5, "normal", defence, "normal", sx, sy, tx, ty);
    deleteItem(sx, sy, "genie2");
		return true;
	
	-- genie2 show room
	elseif idx == 7 then
		showRoom(tx, ty);
    deleteItem(sx, sy, "genie1");
		return true;

	-- swift wind
	elseif idx == 8 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    local move = getCreatureProperty(tx, ty, "move");
    setCreatureProperty(tx, ty, "move", move*2);
		
		function restoreMove(tx, ty)
    	local move = getCreatureProperty(tx, ty, "move");
    	setCreatureProperty(tx, ty, "move", move/2);
		end
		addEntry("endTurn", tx, ty, restoreMove);
		
		return true;

  --tempest
  elseif idx == 9 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    setCreatureProperty(tx, ty, "sleeping", true);

    function awakening(tx, ty)
      console("sleep ended\n");
      setCreatureProperty(tx, ty, "sleeping", false);
    end
    addEntry("endTurn", tx, ty, awakening);
    return true;

  --sleep
  elseif idx == 10 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    function mindwakeup(tx, ty)
      local mind = getCreatureProperty(tx, ty, "mind");
      local awake = false;
      for i = 1, mind, 1 do
        local die = dice("normal", 1);
        if die == 6 then
          awake = true;
          break;
        end
      end
      if awake then
        setCreatureProperty(tx, ty, "sleeping", false);
      else
        setCreatureProperty(tx, ty, "sleeping", true);
        addEntry("startTurn", tx, ty, mindwakeup);
      end
    end
    mindwakeup(tx, ty);
    return true;

  --veil of mist
  elseif idx == 11 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    --spoken to himself
    if sx == tx and sy == ty then
      enableHeroMonsterCollision(false);
    else
      --disable only when it's his turn
      function disableHeroMonsterCollision(tx,ty)
        enableHeroMonsterCollision(false);
      end
      addEntry("startTurn", tx, ty, disableHeroMonsterCollision);
    end
      
    function restoreHeroMonsterCollision(tx,ty)
      enableHeroMonsterCollision(true);
    end
    addEntry("endTurn", tx, ty, restoreHeroMonsterCollision);
   
    
    return true;
  end

	return false;
end
