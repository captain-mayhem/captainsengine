function intro()

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
