function intro()

end

Inv1 = {};
Inv2 = {};
Inv3 = {};
Inv4 = {};

function levelInit()
  if isCreatureAt(5,11) then
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

  if isCreatureAt(6,11) then
    Inv2 = {getItems(6,11)};
    Inv2.name = getCreatureProperty(6, 11, "name");
    Inv2.plyr = getCreatureProperty(6, 11, "player");
    local size = table.getn(Inv2);
    for i=2, size, 1 do
      name = Inv2[i];
      armoryOff(6, 11, name);
      deleteItem(6, 11, name);
    end
    addItem(6, 11, "dagger");
    armory(6, 11, 6, 11, 3);
  end

  if isCreatureAt(5,12) then
    Inv3 = {getItems(5,12)};
    Inv3.name = getCreatureProperty(5, 12, "name");
    Inv3.plyr = getCreatureProperty(5, 12, "player");
    local size = table.getn(Inv3);
    for i=2, size, 1 do
      name = Inv3[i];
      armoryOff(5, 12, name);
      deleteItem(5, 12, name);
    end
    addItem(5, 12, "dagger");
    armory(5, 12, 5, 12, 3);
  end

  if isCreatureAt(6,12) then
    Inv4 = {getItems(6,12)};
    Inv4.name = getCreatureProperty(6, 12, "name");
    Inv4.plyr = getCreatureProperty(6, 12, "player");
    local size = table.getn(Inv4);
    for i=2, size, 1 do
      name = Inv4[i];
      armoryOff(6, 12, name);
      deleteItem(6, 12, name);
    end
    addItem(6, 12, "dagger");
    armory(6, 12, 6, 12, 3);
  end
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

function B(sx, sy)
  setCreatureProperty(sx, sy, "name", "Grak");
  setCreatureProperty(sx, sy, "body", 3);
  setCreatureProperty(sx, sy, "mind", 3);
  addItem(sx, sy, "staff");
  addItem(sx, sy, "wizards_cloak");
end

function C(sx, sy)
  armoryOff(sx,sy,"dagger");
  deleteItem(sx,sy,"dagger");
  local name = getCreatureProperty(sx, sy, "name");
  local plyr = getCreatureProperty(sx, sy, "player");
  if Inv1.name == name and Inv1.plyr == plyr then
    local size = table.getn(Inv1);
    for i=2, size, 1 do
      name = Inv1[i];
      addItem(sx, sy, name);
    end
    Inv1 = {};
    allowSearchAgain(sx, sy);
    output("You get your equipment back", "");
  elseif Inv2.name == name and Inv2.plyr == plyr then
    local size = table.getn(Inv2);
    for i=2, size, 1 do
      name = Inv2[i];
      addItem(sx, sy, name);
    end
    Inv2 = {};
    allowSearchAgain(sx, sy);
    output("You get your equipment back", "");
  elseif Inv3.name == name and Inv3.plyr == plyr then
    local size = table.getn(Inv3);
    for i=2, size, 1 do
      name = Inv3[i];
      addItem(sx, sy, name);
    end
    Inv3 = {};
    allowSearchAgain(sx, sy);
    output("You get your equipment back", "");
  elseif Inv4.name == name and Inv4.plyr == plyr then
    local size = table.getn(Inv4);
    for i=2, size, 1 do
      name = Inv4[i];
      addItem(sx, sy, name);
    end
    Inv4 = {};
    allowSearchAgain(sx, sy);
    output("You get your equipment back", "");
  else
    output("You already got your equipment", "");
  end
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Fimir");
end
