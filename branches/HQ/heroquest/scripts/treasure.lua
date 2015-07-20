treasuretable = {n = 0;}
MAXTREASUREIDX = 24;

function treasureInit()
  math.randomseed(os.time());
  for i=1, MAXTREASUREIDX, 1 do
    table.insert(treasuretable, true);
  end
end

function getTreasureIdx()
  local found = false;
  local idx;
  while not found do
    idx = math.random(table.getn(treasuretable));
    found = treasuretable[idx];
  end
  return idx;
end


function treasure(sx, sy, idx)
  treasuretable[idx] = false;

  -- gems 35
  if idx == 1 or idx == 2 then
    local tofinder = "Tucked into the toe of an old boot, you find a small gem worth 35 gold coins.";
    local toall = getCreatureProperty(sx,sy,"name") .. " found 35 gold.";
    output(tofinder, toall);
    changeMoney(sx, sy, 35);
    return true;

  -- gold 15
  elseif idx == 3 or idx == 4 then
    local tofinder = "Amidst old rags, tattered fur robes and soiled blankets, you find a collection of 15 gold coins.";
    local toall = getCreatureProperty(sx,sy,"name") .. " found 15 gold.";
    output(tofinder, toall);
    changeMoney(sx, sy, 15);
    return true;

  -- gold 25
  elseif idx == 5 or idx == 6 then
    local tofinder = "You find a loose stone and tear it from the wall. Behind it you discover a small leather pouch wrapped in an old rag. Peering inside, you find 25 gold coins."
    local toall = getCreatureProperty(sx,sy,"name") .. " found 25 gold.";
    output(tofinder, toall);
    changeMoney(sx, sy, 25);
    return true;

  -- jewels 50
  elseif idx == 7 or idx == 8 then
    local tofinder = "You find a small wooden box. It is simple-looking and very old. Within you discover that it is lined with velvet and contains very small jewels worth 50 gold coins."
    local toall = getCreatureProperty(sx,sy,"name") .. " found 50 gold.";
    output(tofinder, toall);
    changeMoney(sx, sy, 50);
    return true;

  -- hazard 1
  elseif idx == 9 or idx == 10 then
    local tofinder = "While you are searching, a hidden arrow shoots from the wall striking you. You lose 1 Body Point and your turn is over."
    local toall = getCreatureProperty(sx,sy,"name") .. " was hit by a trap.";
    output(tofinder, toall);
    attack(1, "damage", 0, "normal", sx, sy, sx, sy);
    forceEndTurn();
    treasuretable[idx] = true;
    return true;
  
  -- hazard 2
  elseif idx == 11 or idx == 12 then
    local tofinder = "Suddenly, the stone beneath your feet gives way. You fall into a shallow hole, losing 1 Body Point and ending your turn."
    local toall = getCreatureProperty(sx,sy,"name") .. " was hit by a trap.";
    output(tofinder, toall);
    attack(1, "damage", 0, "normal", sx, sy, sx, sy);
    forceEndTurn();
    treasuretable[idx] = true;
    return true;
  
  -- wandering monster
  elseif idx == 13 or idx == 14 or idx == 15 or idx == 16 or idx == 17 or idx == 18 then
    wandering_monster(sx, sy);
    treasuretable[idx] = true;
    return true;
  
  -- potion of healing
  elseif idx == 19 or idx == 20 or idx == 21 then
    addItem(sx, sy, "potion_of_healing");
    local tofinder = "In a bundle of rags, you find a small bottle of bluish liquid that is a potion of healing."
    local toall = getCreatureProperty(sx,sy,"name") .. " found a potion of healing.";
    output(tofinder, toall);
    return true;
  
  -- heroic brew
  elseif idx == 22 then
    addItem(sx, sy, "heroic_brew");
    local tofinder = "You are surprised to find a leather bag hanging on the wall. It contains a heroic brew."
    local toall = getCreatureProperty(sx,sy,"name") .. " found a heroic brew.";
    output(tofinder, toall);
    return true;
  
  -- potion of defense
  elseif idx == 23 then
    addItem(sx, sy, "potion_of_defense");
    local tofinder = "Amidst a collection of old bottles you find a small vial containing a clear liquid that is a potion of defense."
    local toall = getCreatureProperty(sx,sy,"name") .. " found a potion of defense.";
    output(tofinder, toall);
    return true;
  
  -- potion of strength
  elseif idx == 24 then
    addItem(sx, sy, "potion_of_strength");
    local tofinder = "You find a small purple flask. This strange smelling liquid is a potion of strength."
    local toall = getCreatureProperty(sx,sy,"name") .. " found a potion of strength.";
    output(tofinder, toall);
    return true;
    
  end

	return false;
end
