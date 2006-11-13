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
  
	-- ball of flame (fire) and chaos ball of flame (chaos)
	elseif idx == 4 or idx == 16 then
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
	
	-- genie1 attack (air)
	elseif idx == 6 then
		local defence = getCreatureProperty(tx, ty, "defence");
		attack(5, "normal", defence, "normal", sx, sy, tx, ty);
    deleteItem(sx, sy, "genie2");
		return true;
	
	-- genie2 show room (air)
	elseif idx == 7 then
		showRoom(tx, ty);
    deleteItem(sx, sy, "genie1");
		return true;

	-- swift wind (air)
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

  --tempest (air) and chaosTempest (chaos)
  elseif idx == 9 or idx == 15 then
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

  --sleep (water) and chaosSleep (chaos)
  elseif idx == 10 or idx == 14 then
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

  --veil of mist (water)
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

  --fear (chaos)
  elseif idx == 13 then
    if not isVisible(sx, sy, tx, ty) then
      return false;
    end
    local atta = getCreatureProperty(tx, ty, "attack")-1;
    setCreatureProperty(tx, ty, "attack", 1);
    function attackRestoreFear(tx, ty)
      local mind = getCreatureProperty(tx, ty, "mind");
      local broken = false;
      for i = 1, mind, 1 do
        local die = dice("normal", 1);
        if die == 6 then
          broken = true;
          break;
        end
      end
      if broken then
        output("Fear is broken","Fear is broken");
        local att = getCreatureProperty(tx, ty, "attack");
        setCreatureProperty(tx, ty, "attack", att+atta);
      else
        addEntry("startTurn", tx, ty, attackRestoreFear);
      end
    end
    addEntry("startTurn", tx, ty, attackRestoreFear);
    return true;

  --firestorm (chaos)
  elseif idx == 17 then
    local positions = {getVisibleCreatures(sx, sy)};
    local size = table.getn(positions);
    for i=1, size, 2 do
      local posx = positions[i];
      local posy = positions[i+1];
      attack(3, "damage", 2, "white", sx, sy, posx, posy);
    end
    return true;

  --summon Orcs (chaos)
  elseif idx == 18 then
    local number = dice("normal", 1);
    local num;
    if number < 4 then
      num = 4;
    elseif number > 3 and number < 6 then
      num = 5;
    else
      num = 6;
    end
    --generate num Orcs
    for i=1, num, 1 do
      local adjacent;
      local posx;
      local posy;
      adjacent, posx, posy = getAdjacentFreeField(sx, sy);
      if posx == -1 and posy == -1 then
        return true;
      end
      setMonster(posx,posy,"Orc");
    end
    return true;

  end
	return false;
end
