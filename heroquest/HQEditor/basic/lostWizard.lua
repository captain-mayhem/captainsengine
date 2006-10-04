function intro()
  local text = "The Emperor's personal wizard, Wardoz, has disappeared. "..
    "The Emperor fears that he has been murdered or has succumbed to the "..
    "lures of Chaos magic. You must find out what happened to Wardoz. You "..
    "will each be paid 100 gold coins upon returning to the stairway.";
  messageBox(text);
end


function levelInit()

end

WardozFateKnown = false;

function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  elseif script == "B" or script == "E" then
    return "OnInit";
  elseif script == "C" or script == "n" or script == "N" or script == "D" then
    return "OnTreasure";
  else
    return "None";
  end
end



function A(sx, sy)
  if WardozFateKnown then
    output("You win", "Won");
    changeMoney(sx, sy, 100);
    winHero(sx, sy);
    forceEndTurn(sx, sy);
    return false;
  else
    output("You must find out what happened to Wardoz first","");
    return false;
  end
end

function B(x, y)
  local def = getCreatureProperty(x, y, "defense");
  setCreatureProperty(x, y, "defense", def+1);
end

function C(x, y)
  addItem(x, y, "borins_armor");
  output("You found Borin's armor", "Borin's armor found");
end

function n(x, y)
  --poison needle
  output("You sprung a poison needle", "");
  attack(2, "damage", 0, "normal", x, y, x, y);
  N(x, y);
end

function N(x, y)
  addItem(x, y, purpleLiquid);
  output("You find a purple liquid", "");
end

function D(x, y)
  output("You found 144 gold", "Gold was found.");
  changeMoney(x, y, 144);
  local txt = "Papers are found that prove that Wardoz was consumed by his chaotic magic and turned into a zombie.";
  output(txt, txt);
  WardozFateKnown = true;
end

function E(x, y)
  setCreatureProperty(x, y, "name", "Wardoz");
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Mummy");
end
