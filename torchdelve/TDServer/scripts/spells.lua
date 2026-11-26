-- The spells of data/spells.dat, by their id: stone, ember, gale and tide for the
-- heroes, shadow for Umbrath. Returns false if the spell can't be cast on the target.

-- where shadowstep takes the caster (a quest may set it)
SHADOWSTEP_X = 12;
SHADOWSTEP_Y = 9;

-- gives back up to points of property (body or mind), not over its maximum
local function restore(tx, ty, property, points)
  local value = getCreatureProperty(tx, ty, property) + points;
  local maximum = getCreatureProperty(tx, ty, "max" .. property);
  if value > maximum then
    value = maximum;
  end
  setCreatureProperty(tx, ty, property, value);
end

-- one die per point of will; true if one shows a 6
local function mindBreaks(tx, ty)
  local mind = getCreatureProperty(tx, ty, "mind");
  for i = 1, mind, 1 do
    if dice("normal", 1) == 6 then
      return true;
    end
  end
  return false;
end

function spell(sx, sy, tx, ty, idx)

  -- the target must be in sight, except for far sight and the spells around
  -- the caster (ash storm, pack call, shadowstep)
  local aroundCaster = idx == 7 or idx == 17 or idx == 18 or idx == 19;
  if not aroundCaster and not isVisible(sx, sy, tx, ty) then
    return false;
  end

  -- stonemend (stone): 3 health and 1 will
  if idx == 0 then
    restore(tx, ty, "body", 3);
    restore(tx, ty, "mind", 1);
    return true;

  -- wellspring (tide): 4 health
  elseif idx == 12 then
    restore(tx, ty, "body", 4);
    return true;

  -- stone hide (stone): 2 more defence dice until hit
  elseif idx == 1 then
    local defence = getCreatureProperty(tx, ty, "defence");
    setCreatureProperty(tx, ty, "defence", defence+2);

    function restoreStoneHide(tx, ty)
      local def = getCreatureProperty(tx, ty, "defence");
      setCreatureProperty(tx, ty, "defence", def-2);
    end
    addEntry("hit", tx, ty, restoreStoneHide);
    return true;

  -- wall walk (stone): through walls on the next move
  elseif idx == 2 then
    --cast on oneself
    if sx == tx and sy == ty then
      enableWallCollision(false);
    else
      --only in the target's own turn
      function disableWallCollision(tx, ty)
        enableWallCollision(false);
      end
      addEntry("startTurn", tx, ty, disableWallCollision);
    end

    function restoreWallCollision(tx, ty)
      enableWallCollision(true);
    end
    addEntry("endTurn", tx, ty, restoreWallCollision);
    return true;

  -- cinder dart (ember): 1 damage, no defence
  elseif idx == 3 then
    attack(1, "damage", 0, "normal", sx, sy, tx, ty);
    return true;

  -- flame burst (ember): 3 damage, 3 defence dice
  elseif idx == 4 then
    attack(3, "damage", 3, "normal", sx, sy, tx, ty);
    return true;

  -- witchfire (shadow): 2 damage, 2 defence dice
  elseif idx == 16 then
    attack(2, "damage", 2, "normal", sx, sy, tx, ty);
    return true;

  -- bold flame (ember): 1 more attack and defence die while a monster is in sight
  elseif idx == 5 then
    if isMonsterVisible(tx, ty) then
      setCreatureProperty(tx, ty, "attack", getCreatureProperty(tx, ty, "attack")+1);
      setCreatureProperty(tx, ty, "defence", getCreatureProperty(tx, ty, "defence")+1);

      function restoreBoldFlame(tx, ty)
        if isMonsterVisible(tx, ty) then
          addEntry("move", tx, ty, restoreBoldFlame);
        else
          setCreatureProperty(tx, ty, "attack", getCreatureProperty(tx, ty, "attack")-1);
          setCreatureProperty(tx, ty, "defence", getCreatureProperty(tx, ty, "defence")-1);
        end
      end
      addEntry("move", tx, ty, restoreBoldFlame);
    end
    return true;

  -- thunderclap (gale): an attack with 4 dice
  elseif idx == 6 then
    local defence = getCreatureProperty(tx, ty, "defence");
    attack(4, "normal", defence, "normal", sx, sy, tx, ty);
    return true;

  -- far sight (gale): shows any room
  elseif idx == 7 then
    showRoom(tx, ty);
    return true;

  -- tailwind (gale): twice the movement dice on the next move
  elseif idx == 8 then
    local move = getCreatureProperty(tx, ty, "move");
    setCreatureProperty(tx, ty, "move", move*2);

    function restoreTailwind(tx, ty)
      local move = getCreatureProperty(tx, ty, "move");
      setCreatureProperty(tx, ty, "move", move/2);
    end
    addEntry("endTurn", tx, ty, restoreTailwind);
    return true;

  -- downdraft (gale) and black gust (shadow): the next turn is lost
  elseif idx == 9 or idx == 15 then
    setCreatureProperty(tx, ty, "sleeping", true);

    function awakening(tx, ty)
      setCreatureProperty(tx, ty, "sleeping", false);
    end
    addEntry("endTurn", tx, ty, awakening);
    return true;

  -- drowse (tide) and nightfall (shadow): asleep until a 6 on the mind dice
  elseif idx == 10 or idx == 14 then
    function mindWakeUp(tx, ty)
      if mindBreaks(tx, ty) then
        setCreatureProperty(tx, ty, "sleeping", false);
      else
        setCreatureProperty(tx, ty, "sleeping", true);
        addEntry("startTurn", tx, ty, mindWakeUp);
      end
    end
    mindWakeUp(tx, ty);
    return true;

  -- mistwalk (tide): through the enemies' fields on the next move
  elseif idx == 11 then
    --cast on oneself
    if sx == tx and sy == ty then
      enableHeroMonsterCollision(false);
    else
      --only in the target's own turn
      function disableHeroMonsterCollision(tx, ty)
        enableHeroMonsterCollision(false);
      end
      addEntry("startTurn", tx, ty, disableHeroMonsterCollision);
    end

    function restoreHeroMonsterCollision(tx, ty)
      enableHeroMonsterCollision(true);
    end
    addEntry("endTurn", tx, ty, restoreHeroMonsterCollision);
    return true;

  -- dread (shadow): 1 attack die until a 6 on the mind dice
  elseif idx == 13 then
    local lost = getCreatureProperty(tx, ty, "attack")-1;
    setCreatureProperty(tx, ty, "attack", 1);
    function dreadBreaks(tx, ty)
      if mindBreaks(tx, ty) then
        output("The dread is gone.", "The dread is gone.");
        local att = getCreatureProperty(tx, ty, "attack");
        setCreatureProperty(tx, ty, "attack", att+lost);
      else
        addEntry("startTurn", tx, ty, dreadBreaks);
      end
    end
    addEntry("startTurn", tx, ty, dreadBreaks);
    return true;

  -- ash storm (shadow): 3 damage to everyone in sight, 2 defence dice
  elseif idx == 17 then
    local positions = {getVisibleCreatures(sx, sy)};
    for i = 1, #positions, 2 do
      attack(3, "damage", 2, "torch", sx, sy, positions[i], positions[i+1]);
    end
    return true;

  -- pack call (shadow): three to five goblins around the caster
  elseif idx == 18 then
    local die = dice("normal", 1);
    local count = 3;
    if die == 6 then
      count = 5;
    elseif die >= 3 then
      count = 4;
    end
    for i = 1, count, 1 do
      local adjacent, posx, posy = getAdjacentFreeField(sx, sy);
      if posx == -1 and posy == -1 then
        return true;
      end
      setMonster(posx, posy, "Goblin");
    end
    return true;

  -- shadowstep (shadow): away to the quest's hiding place
  elseif idx == 19 then
    moveObject(sx, sy, SHADOWSTEP_X, SHADOWSTEP_Y, false);
    return true;

  end
  return false;
end
