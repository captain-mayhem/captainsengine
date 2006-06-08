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
end


function wandering_monster(sx, sy)
  default_wandering_monster(sx, sy, "Orc");
end
