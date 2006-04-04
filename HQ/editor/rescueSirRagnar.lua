function intro()
  local text = "Sir Ragnar, one of the Emperor's most powerful knights, has been captured. There is" ..
    " reason to believe that he is being held prisoner by Ulag, the Orc Warlord. You are to find " ..
    "Sir Ragnar and bring him back to the stairway. Prince Magnus will pay 240 gold coins to be " ..
    "divided among the heroes, if they rescue Sir Ragnar. No reward will be paid if Sir Ragnar is " ..
    "killed during the escape."
end

SirRagnarFound = false;
SirRagnarDead = false;
SirRagnarFreed = false;
FirstTime = true;
Amount = 0;

function levelInit()
  function SirRagnarHit(tx, ty)
    local body = getCreatureProperty(tx, ty, "body");
    if body <= 0 then
      SirRagnarDead = true;
    else
      addEntry("hit", tx, ty, SirRagnarHit);
    end
  end
  addEntry("hit", 5, 11, SirRagnarHit);
end

function eventScheduler(script)
  if script == "B" or script == "n" or script == "N" then
    return "OnTreasure";
  elseif script == "C" then
    return "OnOpen";
  elseif script == "D" then
    return "OnInit";
  elseif script == "E" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function n(sx, sy)
  --poison needle
  output("You sprung a poison needle", "");
  attack(1, "damage", 0, "normal", sx, sy, sx, sy);
  output("The chest is empty.", "");
end

function N(sx, sy)
  output("The chest is empty.", "");
end

function B(sx, sy)
  changeMoney(sx, sy, 50);
  addItem(sx, sy, "potion_of_healing");
  tofinder = "You found 50 gold coins and a potion of healing.";
  toall = getCreatureProperty(sx,sy,"name") .. " found 50 gold coins and a potion of healing.";
  output(tofinder,toall);
end


--if the door is opened
function C(sx, sy)
  plyr = getCreatureProperty(sx, sy, "player");
  setCreatureProperty(5, 11, "player", plyr);
  SirRagnarFound = true;
  -- open doors
  openDoor(2, 9, "BOTTOM");
  openDoor(16, 3, "RIGHT");
  openDoor(15, 5, "BOTTOM");
  openDoor(19, 4, "BOTTOM");
  openDoor(20, 6, "RIGHT");
  openDoor(23, 8, "BOTTOM");
  output("An alarm was triggered. Hurry!", "An alarm was triggered. Hurry!");
end

function D(sx, sy)
  setCreatureProperty(sx, sy, "name", "Sir_Ragnar");
  setCreatureProperty(sx, sy, "move", 1);
  setCreatureProperty(sx, sy, "body", 2);
  setCreatureProperty(sx, sy, "attack", 0);
  setCreatureProperty(sx, sy, "defence", 2);
  setCreatureProperty(sx, sy, "diceToMove", true);
end

function E(sx, sy)
  local name = getCreatureProperty(sx, sy, "name");
  if name == "Sir_Ragnar" then
    --Sir Ragnar is freed, let him disappear
    SirRagnarFreed = true;
    removeCreature(sx, sy);
    forceEndTurn();
    return false;
  end
  if not SirRagnarFound then
    output("You have to find Sir Ragnar.", "");
  elseif not SirRagnarFreed and not SirRagnarDead then
    output("Rescue Sir Ragnar first.", "");
  else
    output("You win", "Won");
    --Won, so distribute money
    if (SirRagnarFreed) then
      if (FirstTime) then
        FirstTime = false;
        local num = getNumberOfHeros();
        Amount = 240 / num;
      end
      changeMoney(sx, sy, Amount);
    end
    winHero(sx, sy);
    forceEndTurn(sx, sy);
  end
  return false;
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
