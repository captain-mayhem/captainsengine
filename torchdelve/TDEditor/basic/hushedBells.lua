-- The Hall of Hushed Bells: the seventh quest of Torchdelve.
-- The ember-heart lies under the throne (H: search the throne room, which only secret
-- doors lead into). Taking it rings the bells and calls Ashen Knights. Then back to the
-- forecourt's stair (A).

function intro()
  local text = "The Ashen Court meets in a palace cut into the rock, where bells hang that " ..
    "have never rung. The ember-heart of the Hearthflame is kept in the throne room of the " ..
    "Ashen Chancellor, and the throne room has no door. Those who serve the Court say the " ..
    "rooms of the bell-ringers know the way in. Take the heart and bring it back to the " ..
    "forecourt. Mind the bells.";
  messageBox(text);
end

heartFound = false;

function levelInit()
end

function eventScheduler(script)
  if script == "N" then
    return "OnInit";
  elseif script == "H" then
    return "OnTreasure";
  elseif script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function N(sx, sy)
  setCreatureProperty(sx, sy, "name", "Ashen_Chancellor");
end

function callKnight(sx, sy)
  local free, tx, ty = getAdjacentFreeField(sx, sy);
  if tx ~= -1 then
    setMonster(tx, ty, "Ashen_Knight");
  end
end

function H(sx, sy)
  heartFound = true;
  local txt = "Under the throne's cushion lies the ember-heart, warm as a living coal. As you " ..
    "lift it, every bell in the palace rings at once, and Ashen Knights step out of the " ..
    "shadows beside you.";
  output(txt, getCreatureProperty(sx, sy, "name") .. " took the ember-heart. The bells ring!");
  callKnight(sx, sy);
  callKnight(sx, sy);
end

function A(sx, sy)
  if not heartFound then
    output("You can't leave without the ember-heart.", "");
    return false;
  end
  changeMoney(sx, sy, 100);
  output("You escape the palace with the ember-heart. The Wardens pay 100 gold.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
