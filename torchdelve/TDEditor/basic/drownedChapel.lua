-- The Drowned Chapel: the third quest of Torchdelve.
-- Find the Grey Abbot's ledger (V, searching the vestry), then leave by the bell tower (A).

function intro()
  local text = "The scorched letter led you to the Chapel of Saint Ember, which sank into the " ..
    "flooded mines a hundred years ago. Its last abbot, called the Grey Abbot, is said to " ..
    "still keep his accounts down there. As you wade in through the porch, the rotten roof " ..
    "gives way behind you. The only way out now is up the bell tower in the north-west. " ..
    "Find the Abbot's ledger: it will tell who buys the town's salt with shadow.";
  messageBox(text);
end

ledgerFound = false;

function levelInit()
end

function eventScheduler(script)
  if script == "W" then
    return "OnInit";
  elseif script == "V" then
    return "OnTreasure";
  elseif script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

-- the Grey Abbot: dead for a century and still at his desk
function W(sx, sy)
  setCreatureProperty(sx, sy, "name", "Grey_Abbot");
  setCreatureProperty(sx, sy, "body", 4);
  setCreatureProperty(sx, sy, "attack", 4);
end

function V(sx, sy)
  ledgerFound = true;
  changeMoney(sx, sy, 50);
  local txt = "Behind the vestments, wrapped in oilcloth: the Abbot's ledger and a purse of " ..
    "50 gold. The last page reads: 'The ember-heart, taken from the Hearthflame, sold to the " ..
    "Ashen Court for a hundred years of night. Paid in full.'";
  output(txt, getCreatureProperty(sx, sy, "name") .. " found the Abbot's ledger.");
end

function A(sx, sy)
  if not ledgerFound then
    output("You can't leave without the Abbot's ledger.", "");
    return false;
  end
  output("You climb the bell tower into the night air.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Zombie");
end
