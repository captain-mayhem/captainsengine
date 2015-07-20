function intro()
  local text = "This will be your final test before you set forth. Be sure to use all the skills " ..
    "that you have been taught. You must attempt to find your way out. Several monsters have been " ..
    "placed in the Maze. They will try to stop you, so take care. Whoever finds his way out first " ..
    "will be rewarded with 100 gold coins. This may be used to buy equipment for the adventures to "..
    "come.";
end


function levelInit()

end


function eventScheduler(script)
  if script == "A" then
    return "OnWalkAt";
  else
    return "None";
  end
end

endCount = 0;

function A(sx, sy)
  if endCount == 0 then
    changeMoney(sx, sy, 100);
    output("You finished first and therefore get 100 gold.", "");
  else
    output("You win", "Won");
  end
  winHero(sx, sy);
  endCount = endCount +1;
  forceEndTurn(sx, sy);
  return false;
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
