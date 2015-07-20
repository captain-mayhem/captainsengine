function intro()
  local text = "Long ago, a powerful wizard by the name of Melar created "..
    "a Talisman of Lore which would enhance the wearer's understanding of "..
    "magic. It is rumored that Melar hid the talisman in an underground "..
    "labratory at the heart of his maze, fearing it might be stolen by the "..
    "evil minions of Zargon. As you search for the talisman, beware of "..
    "many traps and deadly monsters.";
  messageBox(text);
end

GargInvictible = true;
TalismanFound = false;


function levelInit()
  function GargHit(tx, ty)
    if GargInvictible then
      --set full life
      setCreatureProperty(tx, ty, "body", 3);
      addEntry("hit", tx, ty, GargHit);
    end
  end
  addEntry("hit", 4, 12, GargHit);
  
  function GargMove(tx, ty)
    GargInvictible = false;
  end
  addEntry("move", 4, 12, GargMove);
  
  setCreatureProperty(4, 12, "sleeping", true);

  addItem(4, 12, "victibleGarg");
  wearArmory(4, 12, "victibleGarg", "right hand", "");
end


function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  elseif script == "B" or script == "D" or script == "E" or script == "N" or script == "n" then
    return "OnTreasure";
  elseif script == "C" then
    return "OnOpen";
  else
    return "None";
  end
end

function A(sx, sy)
  if not TalismanFound then
    output("You have to find the Talisman of Lore", "");
  else
    output("You win", "Won");
    winHero(sx, sy);
    forceEndTurn(sx, sy);
  end
  return false;
end

function B(sx, sy)
  output("You find a half-filled flask sitting on the Alchemist's bench. It is a potion of healing.", "Potion of healing was found");
  addItem(sx, sy, "potion_of_healing");
end

function C(sx, sy)
  setCreatureProperty(4, 12, "sleeping", false);
end

function D(sx, sy)
  output("You find the talisman of lore", "Talisman of Lore found!");
  addItem(sx, sy, "talisman_of_lore");
  TalismanFound = true;
end

function E(sx, sy)
  output("You find Melar's key.","Melar's key was found");
  moveObject(4,7, 4,6, true);
  openDoor(4,7, "RIGHT");
end

function n(sx, sy)
  --poisonous gas
  output("You are hurt by poisonous gas", "Gold was found");
  attack(2, "damage", 0, "normal", sx, sy, sx, sy);
  output("You found 144 gold.", "");
  changeMoney(sx, sy, 144);
end

function N(sx, sy)
  output("You found 144 gold.", "Gold was found");
  changeMoney(sx, sy, 144);
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Zombie");
end
