function intro()

end


function levelInit()
  Inv1 = {getItems(5,11)};
  Inv1.name = getCreatureProperty(5, 11, "name");
  Inv1.plyr = getCreatureProperty(5, 11, "player");
  local size = table.getn(Inv1);
  for i=2, size, 1 do
    name = Inv1[i];
    armoryOff(5, 11, name);
    deleteItem(5, 11, name);
  end
  addItem(5, 11, "dagger");
  armory(5, 11, 5, 11, 3);
end


function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function A(sx, sy)
  output("You win", "Won");
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Fimir");
end
