-- The equipment of data/armory.dat, by its id: the places it takes (one or two),
-- what it adds to the hero, who may use it and, for weapons, how far it reaches:
--   adjacent  the fields next to the hero
--   diagonal  also the diagonal fields
--   thrown    adjacent, or thrown at any creature in sight (and lost)
--   ranged    any creature in sight, but not an adjacent one
-- A hero's attack in data/heros.dat includes the starting weapon, which is
-- worn without calling armory().

MAGE = 3;

EQUIPMENT = {
  [0]  = {name = "trap_tools", places = {"belt"}},
  [1]  = {name = "sidesword",  places = {"right hand"}, attack = 2, noMage = true, reach = "adjacent"},
  [2]  = {name = "iron_helm",  places = {"head"}, defence = 1, noMage = true},
  [3]  = {name = "dirk",       places = {"right hand"}, attack = 1, reach = "thrown"},
  [4]  = {name = "falchion",   places = {"right hand"}, attack = 3, noMage = true, reach = "adjacent"},
  [5]  = {name = "buckler",    places = {"left hand"}, defence = 1, noMage = true},
  [6]  = {name = "oak_staff",  places = {"right hand"}, attack = 1, reach = "diagonal"},
  [7]  = {name = "halberd",    places = {"right hand", "left hand"}, attack = 3, noMage = true, reach = "diagonal"},
  [8]  = {name = "mail_shirt", places = {"body"}, defence = 1, noMage = true},
  [9]  = {name = "arbalest",   places = {"right hand"}, attack = 3, noMage = true, reach = "ranged"},
  [10] = {name = "great_axe",  places = {"right hand", "left hand"}, attack = 4, noMage = true, reach = "adjacent"},
  [11] = {name = "full_plate", places = {"body"}, defence = 2, move = -1, noMage = true},
  [12] = {name = "lamp_charm", places = {"breast"}, mind = 1},
  [13] = {name = "star_cloak", places = {"body"}, defence = 1, onlyMage = true},
  [14] = {name = "emberplate", places = {"body"}, defence = 2, noMage = true},
  [15] = {name = "hand_axe",   places = {"right hand"}, attack = 2, noMage = true, reach = "adjacent"},
};

-- the equipment by name too (armoryOff gets either)
EQUIPMENT_BY_NAME = {};
for id, piece in pairs(EQUIPMENT) do
  EQUIPMENT_BY_NAME[piece.name] = id;
end

-- "great_axe" -> "Great Axe"
function displayName(name)
  local text = string.gsub(name, "_", " ");
  text = string.gsub(text, "(%a)([%w']*)", function(first, rest) return string.upper(first) .. rest; end);
  return text;
end

-- adds sign times the piece's bonuses to the creature
local function applyBonus(sx, sy, piece, sign)
  if piece.attack then
    setCreatureProperty(sx, sy, "attack", getCreatureProperty(sx, sy, "attack") + sign*piece.attack);
  end
  if piece.defence then
    setCreatureProperty(sx, sy, "defence", getCreatureProperty(sx, sy, "defence") + sign*piece.defence);
  end
  if piece.move then
    setCreatureProperty(sx, sy, "move", getCreatureProperty(sx, sy, "move") + sign*piece.move);
  end
  if piece.mind then
    setCreatureProperty(sx, sy, "maxmind", getCreatureProperty(sx, sy, "maxmind") + sign*piece.mind);
    setCreatureProperty(sx, sy, "mind", getCreatureProperty(sx, sy, "mind") + sign*piece.mind);
  end
end

-- puts on the piece with the id idx; false if it can't be worn now
function armory(sx, sy, tx, ty, idx)
  local piece = EQUIPMENT[idx];
  if piece == nil then
    return false;
  end
  local isMage = getCreatureProperty(sx, sy, "id") == MAGE;
  if piece.noMage and isMage then
    output("The Mage cannot use the " .. displayName(piece.name) .. ".", "");
    return false;
  end
  if piece.onlyMage and not isMage then
    output("Only the Mage can use the " .. displayName(piece.name) .. ".", "");
    return false;
  end
  for i, place in ipairs(piece.places) do
    if getArmory(sx, sy, place) ~= -1 then
      if #piece.places == 2 then
        output("You need both hands free for the " .. displayName(piece.name) .. ".", "");
      else
        output("Your " .. place .. " has to be free.", "");
      end
      return false;
    end
  end
  wearArmory(sx, sy, piece.name, piece.places[1], piece.places[2] or "");
  applyBonus(sx, sy, piece, 1);
  return true;
end

-- takes off the piece idx (its id or its name)
function armoryOff(sx, sy, idx)
  if type(idx) == "string" then
    idx = EQUIPMENT_BY_NAME[idx];
  end
  local piece = EQUIPMENT[idx];
  if piece == nil then
    return;
  end
  takeOff(sx, sy, piece.name);
  applyBonus(sx, sy, piece, -1);
end

-- is called when attacking; returns the number of attack dice, or -1 if the
-- weapon doesn't reach the target. idx is -1 for bare hands.
function use_weapon(sx, sy, tx, ty, idx)
  local reach = "adjacent";
  if idx ~= -1 then
    local piece = EQUIPMENT[idx];
    if piece == nil or piece.reach == nil then
      return -1;
    end
    reach = piece.reach;
  end
  if not isVisible(sx, sy, tx, ty) then
    return -1;
  end
  local adjacent = WisAdjacent(sx, sy, tx, ty);
  local attack = getCreatureProperty(sx, sy, "attack");
  if reach == "adjacent" then
    if adjacent then return attack; end
  elseif reach == "diagonal" then
    if adjacent or WisDiagonal(sx, sy, tx, ty) then return attack; end
  elseif reach == "thrown" then
    if not adjacent then
      -- the thrown weapon is gone
      local piece = EQUIPMENT[idx];
      armoryOff(sx, sy, idx);
      deleteItem(sx, sy, piece.name);
    end
    return attack;
  elseif reach == "ranged" then
    if not adjacent and not WisDiagonal(sx, sy, tx, ty) then return attack; end
  end
  return -1;
end

--helper functions for use_weapon
function WisAdjacent(sx, sy, tx, ty)
  if sx == tx then
    return sy+1 == ty or sy-1 == ty;
  elseif sy == ty then
    return sx+1 == tx or sx-1 == tx;
  end
  return false;
end

function WisDiagonal(sx, sy, tx, ty)
  return (sx+1 == tx or sx-1 == tx) and (sy+1 == ty or sy-1 == ty);
end
