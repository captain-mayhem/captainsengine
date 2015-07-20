function intro()
  local text = "Prince Magnus has ordered that the Orc Warlord, Ulag, "..
    "who was responsible for the imprisonment of Sir Ragnar, be sought "..
    "out and destroyed. When Ulag is destroyed, the heros will receive "..
    "a reward of 180 gold coins to be divided among them. Any treasure "..
    "found in Ulag's stronghold may be kept by the finder alone."
  messageBox(text);
end

UlagDead = false;
FirstTime = true;
Amount = 0;

function levelInit()
  function UlagHit(tx, ty)
    local body = getCreatureProperty(tx, ty, "body");
    if body <= 0 then
      UlagDead = true;
    else
      addEntry("hit", tx, ty, UlagHit);
    end
  end
  addEntry("hit", 6, 14, UlagHit);
end

function eventScheduler(script)
  if script == "C" then
    return "OnWalkAt";
  elseif script == "A" or script == "B" then
    return "OnTreasure";
  elseif script == "D" then
    return "OnInit";
  else
    return "None";
  end
end

function A(sx, sy)
  addItem(sx, sy, "staff");
  output("You found a staff", "");
end

function B(sx, sy)
  changeMoney(sx, sy, 24);
  addItem(sx, sy, "potion_of_healing");
  output("You found 24 gold and a potion of healing", "");
end

function C(sx, sy)
  if not UlagDead then
    output("You must kill Ulag first.", "");
    return false;
  end
  output("You win", "Won");
  --Won, so distribute money
  if (FirstTime) then
    FirstTime = false;
    local num = getNumberOfHeros();
    Amount = 180 / num;
  end
  changeMoney(sx, sy, Amount);
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function D(sx, sy)
  setCreatureProperty(sx, sy, "name", "Ulag");
  setCreatureProperty(sx, sy, "move", 10);
  setCreatureProperty(sx, sy, "body", 2);
  setCreatureProperty(sx, sy, "attack", 4);
  setCreatureProperty(sx, sy, "defence", 5);
  setCreatureProperty(sx, sy, "mind", 3);
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
