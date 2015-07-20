function intro()

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

function A(sx, sy)
  output("You win", "Won");
  winHero(sx, sy);
  forceEndTurn(sx, sy);
  return false;
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
