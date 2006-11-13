function armory(sx, sy, tx, ty, idx)

  -- tool kit
  if idx == 0 then
    local id = getArmory(sx, sy, "belt");
    if id == -1 then
      wearArmory(sx, sy, "tool_kit", "belt", "");
      return true
    end
    return false;
  end

  --shortsword
  if idx == 1 then
    --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "shortsword", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+2);
      return true;
    else
      err(sx, sy, "Your right hand has to be free");
      return false;
    end

  --helmet
  elseif idx == 2 then
    --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "head");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "helmet", "head", "");
      local def = getCreatureProperty(sx, sy, "defence");
      setCreatureProperty(sx, sy, "defence", def+1);
      return true;
    else
      err(sx, sy, "Your head has to be free");
      return false;
    end

  
  --dagger
  elseif idx == 3 then
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "dagger", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+1);
      return true;
    else
      output("Your right hand has to be free", "");
      return false;
    end
  
  --broadsword
  elseif idx == 4 then
    --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "broadsword", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+3);
      return true;
    else
      output("Your right hand has to be free", "");
      return false;
    end

  --shield
  elseif idx == 5 then
   --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "left hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "shield", "left hand", "");
      local def = getCreatureProperty(sx, sy, "defence");
      setCreatureProperty(sx, sy, "defence", def+1);
      return true;
    else
      err(sx, sy, "Your left hand has to be free");
      return false;
    end

    
  --staff
  elseif idx == 6 then
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "staff", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+1);
      return true;
    else
      output("Your right hand has to be free", "");
      return false;
    end

    
  --longsword
  elseif idx == 7 then
    --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "longsword", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+3);
      return true;
    else
      output("Your right hand has to be free", "");
      return false;
    end


  --chain mail
  elseif idx == 8 then
    --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "body");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "chain_mail", "body", "");
      local def = getCreatureProperty(sx, sy, "defence");
      setCreatureProperty(sx, sy, "defence", def+1);
      return true;
    else
      err(sx, sy, "Your body has to be free");
      return false;
    end

  --crossbow
  elseif idx == 9 then
     --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "right hand");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "crossbow", "right hand", "");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+3);
      return true;
    else
      output("Your right hand has to be free", "");
      return false;
    end


  --battle axe
  elseif idx == 10 then
     --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id1 = getArmory(sx, sy, "right hand");
    local id2 = getArmory(sx, sy, "left hand");
    --free?
    if id1 == -1 and id2 == -1 then
      wearArmory(sx, sy, "battle_axe", "right hand", "left hand");
      local atta = getCreatureProperty(sx, sy, "attack");
      setCreatureProperty(sx, sy, "attack", atta+4);
      return true;
    else
      output("Both of your hands have to be free", "");
      return false;
    end

    
  --plate mail
  elseif idx == 11 then
     --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "body");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "plate_mail", "body", "");
      local def = getCreatureProperty(sx, sy, "defence");
      setCreatureProperty(sx, sy, "defence", def+2);
      local mov = getCreatureProperty(sx, sy, "move");
      setCreatureProperty(sx, sy, "move", mov-1);
      return true;
    else
      err(sx, sy, "Your body has to be free");
      return false;
    end
    
  -- talisman of lore
  elseif idx == 12 then
    local id = getArmory(sx, sy, "breast");
    if id == -1 then
      wearArmory(sx, sy, "talisman_of_lore", "breast", "");
      local maxmind = getCreatureProperty(sx, sy, "maxmind");
      setCreatureProperty(sx, sy, "maxmind", maxmind+1);
      local mind = getCreatureProperty(sx, sy, "mind");
      setCreatureProperty(sx, sy, "mind", mind+1);
      return true;
    end
    output("Your breast has to be free","");
    return false;
    
  -- wizard's cloak
  elseif idx == 13 then
    --is it the wizard?
    if not (getCreatureProperty(sx, sy, "id") == 3) then
      return false;
    end
    local id = getArmory(sx, sy, "body");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "wizards_cloak", "body", "");
      local def = getCreatureProperty(sx, sy, "defence");
      setCreatureProperty(sx, sy, "defence", def+1);
      return true;
    else
      err(sx, sy, "Your body has to be free");
      return false;
    end

  --borin's armor
  elseif idx == 14 then
     --is it the wizard?
    if getCreatureProperty(sx, sy, "id") == 3 then
      return false;
    end
    local id = getArmory(sx, sy, "body");
    --free?
    if id == -1 then
      wearArmory(sx, sy, "borins_armor", "body", "");
      local def = getCreatureProperty(sx, sy, "defense");
      setCreatureProperty(sx, sy, "defense", def+2);
      return true;
    else
      err(sx, sy, "Your body has to be free");
      return false;
    end
    
  end
  
end


function armoryOff(sx, sy, idx)

  -- tool kit
  if idx == 0 or idx == "tool_kit" then
    takeOff(sx, sy, "tool_kit");

  --shortsword
  elseif idx == 1 or idx == "shortsword" then
    takeOff(sx, sy, "shortsword");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-2);

  --helmet
  elseif idx == 2 or idx == "helmet" then
    takeOff(sx, sy, "helmet");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", def-1);
  
  --dagger
  elseif idx == 3 or idx == "dagger" then
    takeOff(sx, sy, "dagger");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-1);
  
  --broadsword
  elseif idx == 4 or idx == "broadsword" then
    takeOff(sx, sy, "broadsword");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-3);
  
  --shield
  elseif idx == 5 or idx == "shield" then
    takeOff(sx, sy, "shield");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", def-1);
    
  --staff
  elseif idx == 6 or idx == "staff" then
    takeOff(sx, sy, "staff");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-1);

  --longsword
  elseif idx == 7 or idx == "longsword" then
    takeOff(sx, sy, "longsword");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-3);

  --chain mail
  elseif idx == 8 or idx == "chain_mail" then
    takeOff(sx, sy, "chain_mail");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", def-1);

  --crossbow
  elseif idx == 9 or idx == "crossbow" then
    takeOff(sx, sy, "crossbow");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-3);

  --battle axe
  elseif idx == 10 or idx == "battle_axe" then
    takeOff(sx, sy, "battle_axe");
    local atta = getCreatureProperty(sx, sy, "attack");
    setCreatureProperty(sx, sy, "attack", atta-4);

  --plate mail
  elseif idx == 11 or idx == "plate_mail" then
    takeOff(sx, sy, "plate_mail");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", def-2);
    local mov = getCreatureProperty(sx, sy, "move");
    setCreatureProperty(sx, sy, "move", mov+1);

  -- talisman of lore
  elseif idx == 12 or idx == "talisman_of_lore" then
    takeOff(sx, sy, "talisman_of_lore");
    local maxmind = getCreatureProperty(sx, sy, "maxmind");
    setCreatureProperty(sx, sy, "maxmind", maxmind-1);
    local mind = getCreatureProperty(sx, sy, "mind");
    setCreatureProperty(sx, sy, "mind", mind-1);

  --wizards cloak
  elseif idx == 13 or idx == "wizards_cloak" then
    takeOff(sx, sy, "wizards_cloak");
    local def = getCreatureProperty(sx, sy, "defence");
    setCreatureProperty(sx, sy, "defence", def-1);
  
  --borin's armor
  elseif idx == 14 or idx == "borins_armor" then
    takeOff(sx, sy, "borins_armor");
    local def = getCreatureProperty(sx, sy, "defense");
    setCreatureProperty(sx, sy, "defense", def-2);

  end

end

-- is called when attacking, must return the number of attack dices
-- or -1 if the range check fails
function use_weapon(sx, sy, tx, ty, idx)
  --directly adjacent weapons
  if idx == -1 or idx == 1 or idx == 4 or idx == 10 then
    if not isVisible(sx, sy, tx, ty) then
      return -1;
    end
    if WisAdjacent(sx, sy, tx, ty) then
      return getCreatureProperty(sx, sy, "attack");
    else
      return -1;
    end
  --adjacent weapons
  elseif idx == 6 or idx == 7 then
    if not isVisible(sx, sy, tx, ty) then
      return -1;
    end
    if WisAdjacent(sx, sy, tx, ty) or WisDiagonal(sx, sy, tx, ty) then
      return getCreatureProperty(sx, sy, "attack");
    else
      return -1;
    end
  --throwable weapons
  elseif idx == 3 then
    if not isVisible(sx, sy, tx, ty) then
      return -1;
    end
    if not WisAdjacent(sx, sy, tx, ty) then
      local atta = getCreatureProperty(sx, sy, "attack");
      armoryOff(sx, sy, 3);
      deleteItem(sx, sy, "dagger");
      return atta;
    end
    return getCreatureProperty(sx, sy, "attack");
  --range weapons
  elseif idx == 9 then
    if WisAdjacent(sx, sy, tx, ty) or WisDiagonal(sx, sy, tx, ty) then
      return -1;
    end
    if isVisible(sx, sy, tx, ty) then
      return getCreatureProperty(sx, sy, "attack");
    else
      return -1;
    end
  --Gargoyle's weapon in Melar's maze
  elseif idx == 1000 then
    if not isVisible(sx, sy, tx, ty) then
      return -1;
    end
    if WisAdjacent(sx, sy, tx, ty) then
      GargInvictible = false;
      return getCreatureProperty(sx, sy, "attack");
    else
      return -1;
    end

  end
  return -1;
end

--helper functions for use weapon
function WisAdjacent(sx, sy, tx, ty)
  if sx == tx then
    if sy+1 == ty or sy-1 == ty then
      return true;
    end
    return false;
  elseif sy == ty then
    if sx+1 == tx or sx-1 == tx then
      return true;
    end
    return false;
  else
    return false;
  end
end

function WisDiagonal(sx, sy, tx, ty)
  if sx+1 == tx and sy+1 == ty then
    return true
  elseif sx+1 == tx and sy-1 == ty then
    return true
  elseif sx-1 == tx and sy+1 == ty then
    return true
  elseif sx-1 == tx and sy-1 == ty then
    return true
  end
end
