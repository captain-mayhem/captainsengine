function intro()
  local text = "Three treasure chests have been stolen while being taken to "..
    "the Emperor. A reward of 240 gold coins has been offered to any group "..
    "of heroes who return the chests and ALL of the gold. The thieves are "..
    "a well-known band of Orcs whose lair is in the Black Mountains. They "..
    "are led by Gulthor, a Chaos Warrior.";
  messageBox(text);
end

Amount = 0;
FirstTime = true;
NumberChests = 3;
PickedUp = 0;


function levelInit()

end


function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  elseif script == "B" then
    return "OnTreasure";
  else
    return "None";
  end
end

function A(sx, sy)
  --carrying a chest
  if deleteItem(sx, sy, "chest") then
    NumberChests = NumberChests - 1;
    setCreatureProperty(sx, sy, "move", 2);
  end
  -- not all saved
  if NumberChests > 0 then
    output("There are " .. NumberChests .. " chests left to rescue.","");
  else
    output("You win", "Won");
    --Won, so distribute money
    if (FirstTime) then
      FirstTime = false;
      local num = getNumberOfHeros();
      Amount = 240 / num;
    end
    changeMoney(sx, sy, Amount);
    winHero(sx, sy);
    forceEndTurn(sx, sy);
  end
  return false;
end

function B(sx, sy)
  --check if you are already carrying a chest
  if deleteItem(sx, sy, "chest") then
    addItem(sx, sy, "chest");
    output("You can carry only one chest at a time.", "");
    allowSearchAgain(sx, sy);
    return;
  end
  if PickedUp < 3 then
    PickedUp = PickedUp + 1;
    setCreatureProperty(sx, sy, "move", 1);
    output("You are carrying one chest now", "");
    addItem(sx, sy, "chest");
    allowSearchAgain(sx, sy);
  else
    output("No more chests left.", "");
  end
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Fimir");
end
