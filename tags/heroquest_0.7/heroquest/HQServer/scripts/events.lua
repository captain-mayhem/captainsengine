attacktable = {n = 0;}
hittable = {n = 0;}
turntable = {n = 0;}
movetable = {n = 0;}
starttable = {n = 0;}

function attacking(x, y)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  local size = table.getn(attacktable);
  if size < 1 then
    return
  end
  for i=size, 1, -1 do
    if attacktable[i].name == name and attacktable[i].plyr == plyr then
  	  attacktable[i].f(x, y);
  	  table.remove(attacktable, i);
  	end
  end
end

function hit(x, y)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  local size = table.getn(hittable);
  if size < 1 then
    return
  end
  for i=size, 1, -1 do
    if hittable[i].name == name and hittable[i].plyr == plyr then
  	  hittable[i].f(x, y);
  	  table.remove(hittable, i);
  	end
  end
end

function endTurn(x, y)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  local size = table.getn(turntable);
  if size < 1 then
    return
  end
  for i=size, 1, -1 do
    if turntable[i].name == name and turntable[i].plyr == plyr then
  	  turntable[i].f(x, y);
  	  table.remove(turntable, i);
  	end
  end
end

function startTurn(x, y)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  local size = table.getn(starttable);
  if size < 1 then
    return
  end
  for i=size, 1, -1 do
    if starttable[i].name == name and starttable[i].plyr == plyr then
  	  starttable[i].f(x, y);
  	  table.remove(starttable, i);
  	end
  end
end

function move(x, y)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  local size = table.getn(movetable);
  if size < 1 then
    return
  end
  for i=size, 1, -1 do
    if movetable[i].name == name and movetable[i].plyr == plyr then
  	  movetable[i].f(x, y);
  	  table.remove(movetable, i);
  	end
  end
end

function addEntry(tab, x, y, f)
  local name = getCreatureProperty(x, y, "name");
  local plyr = getCreatureProperty(x, y, "player");
  entry = {};
  entry.name = name;
  entry.plyr = plyr;
  entry.f = f;
  if tab == "hit" then
    table.insert(hittable, entry);
  elseif tab == "endTurn" then
    table.insert(turntable, entry);
  elseif tab == "move" then
    table.insert(movetable, entry);
  elseif tab == "startTurn" then
    table.insert(starttable, entry);
  elseif tab == "attack" then
    table.insert(attacktable, entry);
  end
end
