-- The Smugglers' Warren: the fifth quest of Torchdelve.
-- Search Captain Brannoch's cabin for the chart (C): it opens the hidden tunnel to the
-- way out (A). Brannoch (B) carries a bounty.

function intro()
  local text = "The Ashen Court buys its salt from smugglers who row it through a sea cave into " ..
    "a warren of tunnels under the cliffs. The heroes slip in on a salt barge; behind them " ..
    "the tide closes the cave mouth. Captain Brannoch, an armoured brute who has never lost " ..
    "a fight, keeps the warren's chart in his cabin. It shows the hidden tunnel to the road " ..
    "inland. The Wardens pay a bounty of 50 gold for Brannoch.";
  messageBox(text);
end

chartFound = false;
brannochDead = false;
bountyPaid = false;

function levelInit()
end

function eventScheduler(script)
  if script == "B" then
    return "OnInit";
  elseif script == "C" then
    return "OnTreasure";
  elseif script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

function B(sx, sy)
  setCreatureProperty(sx, sy, "name", "Captain_Brannoch");
  setCreatureProperty(sx, sy, "body", 4);
  function brannochHit(tx, ty)
    if getCreatureProperty(tx, ty, "body") <= 0 then
      brannochDead = true;
      output("Captain Brannoch crashes to the ground. The bounty is yours.",
        "Captain Brannoch has fallen.");
    else
      addEntry("hit", tx, ty, brannochHit);
    end
  end
  addEntry("hit", sx, sy, brannochHit);
end

function C(sx, sy)
  chartFound = true;
  openDoor(18, 12, "RIGHT");
  local txt = "Under the captain's charts you find the warren's own: a tunnel behind the " ..
    "east wall of the main passage. You press the marked stone, and the wall swings open.";
  output(txt, getCreatureProperty(sx, sy, "name") .. " found the chart. The hidden tunnel is open.");
end

function A(sx, sy)
  if brannochDead and not bountyPaid then
    bountyPaid = true;
    changeMoney(sx, sy, 50);
    output("You carry Brannoch's helmet to the Wardens: 50 gold bounty.", "");
  end
  output("You climb out into the hills, the warren behind you.", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end

function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
