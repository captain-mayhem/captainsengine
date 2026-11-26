-- What a hero finds when searching a room for treasure and the quest has nothing
-- of its own there: one of MAXTREASUREIDX finds, each used once per quest, except
-- the hazards, the wandering monsters and the empty searches.

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
    idx = math.random(#treasuretable);
    found = treasuretable[idx];
  end
  return idx;
end

local function finderName(sx, sy)
  return getCreatureProperty(sx, sy, "name");
end

local function gold(sx, sy, amount, text)
  output(text, finderName(sx, sy) .. " found " .. amount .. " gold.");
  changeMoney(sx, sy, amount);
end

local function find(sx, sy, name, text)
  addItem(sx, sy, name);
  output(text, finderName(sx, sy) .. " found a " .. displayName(name) .. ".");
end

local function hazard(sx, sy, text)
  output(text, finderName(sx, sy) .. " was hurt while searching.");
  attack(1, "damage", 0, "normal", sx, sy, sx, sy);
  forceEndTurn();
end

function treasure(sx, sy, idx)
  treasuretable[idx] = false;

  if idx == 1 or idx == 2 then
    gold(sx, sy, 20, "Wedged into a crack between two stones: a few coins, 20 gold in all.");
  elseif idx == 3 or idx == 4 then
    gold(sx, sy, 30, "Under a heap of mouldy sacks lies a forgotten purse with 30 gold.");
  elseif idx == 5 then
    gold(sx, sy, 45, "Behind a loose brick: a silver candlestick, black with soot. It is worth 45 gold.");
  elseif idx == 6 then
    gold(sx, sy, 60, "A gold ring glints in the dust, still on a finger bone. It is worth 60 gold.");
  elseif idx == 7 then
    gold(sx, sy, 15, "A Warden's old tinderbox, of good brass. It is worth 15 gold.");

  -- hazards: they stay in the table
  elseif idx == 8 or idx == 9 then
    hazard(sx, sy, "A stone comes loose from the ceiling and strikes your shoulder. You lose 1 health and your turn ends.");
    treasuretable[idx] = true;
  elseif idx == 10 then
    hazard(sx, sy, "The rotten boards under your feet break. You lose 1 health and your turn ends.");
    treasuretable[idx] = true;
  elseif idx == 11 then
    local mind = getCreatureProperty(sx, sy, "mind");
    if mind > 1 then
      setCreatureProperty(sx, sy, "mind", mind-1);
    end
    output("A cloud of grey spores rises from a rotten crate. Your head swims: you lose 1 will.",
      finderName(sx, sy) .. " breathed in spores.");
    treasuretable[idx] = true;

  -- wandering monsters: they stay in the table
  elseif idx >= 12 and idx <= 16 then
    wandering_monster(sx, sy);
    treasuretable[idx] = true;

  -- potions
  elseif idx >= 17 and idx <= 19 then
    find(sx, sy, "hearthbalm", "In a clay pot sealed with wax: a Hearthbalm, still warm to the touch.");
  elseif idx == 20 then
    find(sx, sy, "battle_ale", "A small cask hangs from a hook in the wall. It holds a Battle Ale.");
  elseif idx == 21 then
    find(sx, sy, "iron_tonic", "Among broken bottles one has survived: an Iron Tonic.");
  elseif idx == 22 then
    find(sx, sy, "fire_tonic", "A flask wrapped in red cloth: a Fire Tonic.");
  elseif idx == 23 then
    find(sx, sy, "murky_vial", "A vial of murky liquid without a label. Who knows what it does?");

  -- nothing: it stays in the table
  elseif idx == 24 then
    output("Only dust, cobwebs and the bones of a rat.", "");
    treasuretable[idx] = true;

  else
    return false;
  end
  return true;
end
