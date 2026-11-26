-- Vesk's Furnace: the sixth quest of Torchdelve.
-- Smash the three alembics (P, Q, R: search their rooms); while one still boils, Vesk
-- Morrow's (V) wounds close. Defeat him and return to the stair (A).

function intro()
  local text = "The smugglers' salt goes to Vesk Morrow, once an alchemist of the Lantern " ..
    "Wardens, now the Ashen Court's distiller. In the vaults under his tower he boils salt " ..
    "and shadow into the black oil that makes the Court's servants strong. Three alembics " ..
    "feed his furnace, and while any of them boils, its fumes heal him. Smash them, defeat " ..
    "Vesk and come back to the stair.";
  messageBox(text);
end

alembics = 3;
veskDead = false;

function levelInit()
end

function eventScheduler(script)
  if script == "V" then
    return "OnInit";
  elseif script == "P" or script == "Q" or script == "R" then
    return "OnTreasure";
  elseif script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function V(sx, sy)
  setCreatureProperty(sx, sy, "name", "Vesk_Morrow");
  function veskHit(tx, ty)
    if alembics > 0 then
      setCreatureProperty(tx, ty, "body", 4);
      output("Fumes from the boiling alembics close Vesk's wounds.",
        "Fumes from the boiling alembics close Vesk's wounds.");
      addEntry("hit", tx, ty, veskHit);
    elseif getCreatureProperty(tx, ty, "body") <= 0 then
      veskDead = true;
      output("Vesk Morrow collapses into his own black oil.", "Vesk Morrow is dead.");
    else
      addEntry("hit", tx, ty, veskHit);
    end
  end
  addEntry("hit", sx, sy, veskHit);
end

function smash(sx, sy)
  alembics = alembics - 1;
  local name = getCreatureProperty(sx, sy, "name");
  if alembics > 0 then
    output("You smash the alembic. " .. alembics .. " still boil.",
      name .. " smashed an alembic. " .. alembics .. " left.");
  else
    output("You smash the last alembic. The fumes fade, and Vesk can be hurt now.",
      name .. " smashed the last alembic. Vesk can be hurt now.");
  end
end

function P(sx, sy)
  smash(sx, sy);
end

function Q(sx, sy)
  smash(sx, sy);
end

function R(sx, sy)
  smash(sx, sy);
end

function A(sx, sy)
  if not veskDead then
    output("Vesk Morrow still lives.", "");
    return false;
  end
  changeMoney(sx, sy, 75);
  output("You take 75 gold from Vesk's strongbox and climb out.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Zombie");
end
