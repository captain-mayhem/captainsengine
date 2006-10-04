function intro()

end


function levelInit()

end


function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  elseif script == "B" then
    return "OnInit";
  elseif script == "C" then
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

function B(x, y)
  setCreatureProperty(x, y, "name", "Balur");
  setCreatureProperty(x, y, "move", 8);
  setCreatureProperty(x, y, "attack", 2);
  setCreatureProperty(x, y, "defense", 5);
  setCreatureProperty(x, y, "body", 3);
  setCreatureProperty(x, y, "mind", 7);
  addItem(x, y, "chaosBallOfFlame");
  addItem(x, y, "firestorm");
  addItem(x, y, "chaosTempest");
  addItem(x, y, "summonOrcs");
  addItem(x, y, "fear");
  addItem(x, y, "escape");
end

function C(x, y)
  output("You find 150 gold and the Wand of Magic","Gold and Wand of Magic found");
  changeMoney(x, y, 150);
  addItem("wandOfMagic");
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Fimir");
end
