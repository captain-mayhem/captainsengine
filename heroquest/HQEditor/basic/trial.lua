function intro()
  local text = "You have learned well, my friends. Now has come the time of your first trial. " ..
    "You must first enter the catacombs which contain Fellmarg's tomb. You must seek out and " ..
    "destroy Verag, a foul Gargoyle who hides in the catacombs. This Quest is not easy and you " ..
    "must work together in order to survive. This is your first step on the road to becoming true " ..
    "heros. Tread carefully my friends.";
  messageBox(text);
end

veragDead = false;

function levelInit()
  function killVerag(tx, ty)
    local body = getCreatureProperty(tx, ty, "body");
    if body <= 0 then
      veragDead = true;
    else
      addEntry("hit", tx, ty, killVerag);
    end
  end
  addEntry("hit", 12, 8, killVerag);
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end


function eventScheduler(script)
  if script == "C" or script == "F" then
    return "OnInit";
  elseif script == "A" or script == "B" or script == "D" or script == "E" then
    return "OnTreasure";
  elseif script == "G" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function A(sx, sy)
  output("The weapons on this weapon rack are chipped, rusted and broken. There is nothing here you would want.", "Nothing found");
end

function B(sx, sy)
  output("This treasure chest is empty.", "Nothing found");
end

function C(sx, sy)
  setCreatureProperty(sx, sy, "attack", 4);
end

function D(sx, sy)
  output("You found 84 gold", "");
  changeMoney(sx, sy, 84);
end

function E(sx, sy)
  output("You found 120 gold", "");
  changeMoney(sx, sy, 120);
end

function F(sx, sy)
  setCreatureProperty(sx, sy, "name", "Verag");
end

function G(sx, sy)
  if not veragDead then
    output("You must kill Verag first.", "");
    return false;
  end
  output("You win", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

