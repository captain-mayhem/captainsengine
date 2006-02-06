function intro()

end

NumberChests = 0;

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
  output("You win", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function B(sx, sy)
  setCreatureProperty(sx, sy, "move", 1);
  output("You are carrying one chest now");
  addItem(sx, sy, "staff");
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Fimir");
end
