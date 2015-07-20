--settings
file_path = "C:\\tmp\\"
project_name = "test"

count = 0
view_width = 0
view_height = 0
cam = 0
visibility = {}
pictures = {}
layers = {}
layer_groups = {}

dialog = {}

list_file = nil
objects_file = nil

function traverseLayers(moho, grouplayer, func)
  for i=1,grouplayer:CountLayers() do
    local layer = grouplayer:Layer(i-1)
    traverseLayer(moho, layer, func);
  end
end

function sleep(s)
  local ntime = os.time() + s
  repeat until os.time() > ntime
end

function traverseLayer(moho, layer, func)
  local bbox = layer:Bounds(0)
  if layer:LayerType() == MOHO.LT_GROUP then
    glayer = moho:LayerAsGroup(layer)
    traverseLayers(moho, glayer, func)
  elseif layer:LayerType() == MOHO.LT_BONE then
    blayer = moho:LayerAsBone(layer)
    traverseLayers(moho, blayer, func)
  elseif layer:LayerType() == MOHO.LT_SWITCH then
    slayer = moho:LayerAsSwitch(layer)
    local val = slayer:GetValue(0)
    --print(val)
    local active_layer = slayer:LayerByName(val)
    if active_layer == nil then
      --print("wtf")
      active_layer = slayer:Layer(slayer:CountLayers()-1)
    end
    --print(active_layer:Name())
    traverseLayer(moho, active_layer, func)
  elseif not bbox:IsEmpty() then
    func(moho, layer)
  end
end

function makeInvisble(moho, layer)
  local vis = layer.fVisibility
  visibility[count] = vis:GetValue(0)
  vis:SetValue(0, false)
  count = count + 1
end

function restoreVisibility(moho, layer)
  local vis = layer.fVisibility
  vis:SetValue(0, visibility[count])
  count = count + 1
end

function drawLayers(name, moho, layers)
  --print ("drawLayers")
  local bbox = LM.BBox:new_local()
  for k,v in ipairs(layers) do
    local layer = v
    local vis = layer.fVisibility
    vis:SetValue(0, true)
    local newbox = layer:Bounds(0)
    local mat = LM.Matrix:new_local()
    layer:GetLayerTransform(0, mat, moho.document)
    mat:Transform(newbox)
    bbox:AccumulateBBox(newbox)
  end
  
  local pos = moho.document.fCameraTrack:GetValue(0)
  --print(pos.x.."/"..pos.y.."/"..pos.z)
  pos.x = ((bbox.fMin.x+bbox.fMax.x)/2)
  pos.y = ((bbox.fMin.y+bbox.fMax.y)/2)
  local width = moho:DocToPixel((bbox.fMax.x-bbox.fMin.x)/pos.z)
  local height = moho:DocToPixel((bbox.fMax.y-bbox.fMin.y)/pos.z)
  width = math.ceil(width)
  height = math.ceil(height)
  pos.z = cam.z*(bbox.fMax.y-bbox.fMin.y)/2
  moho.document:SetShape(width, height)
  moho.document.fCameraTrack:SetValue(0, pos)
  
  local filename = file_path.."gfx/"..project_name.."_"..name..".png"
  moho:FileRender(filename)
  sleep(0.3)
  pictures[count] = filename
  list_file:write(filename.."\n")
  --sleep(1)
  --end
  moho.document:SetShape(view_width, view_height)
  moho.document.fCameraTrack:SetValue(0, cam)
  --print("CAM:"..cam.x.."/"..cam.y.."/"..cam.z)
  pos.z = cam.z
  count = count + 1
  
  for k,v in ipairs(layers) do
    local layer = v
    local vis = layer.fVisibility
    vis:SetValue(0, false)
  end
end

function buildMenu(moho, layer)
  layers[count] = layer
  dialog.layers[count] = MOHO.MSG_BASE + count
  dialog.layer_menu:AddItem(layer:Name(), 0, dialog.layers[count])
  if (count == 1) then
    --dialog.layer_menu:SetChecked(dialog.layers[count], true)
  end
  count = count + 1
end

function dialog:HandleMessage(message)
  local check = not dialog.layer_menu:IsChecked(message)
  dialog.layer_menu:SetChecked(message, check)
end

function dialog:new()
  self.layers = {}
  local dia = LM.GUI.SimpleDialog("Export Settings", dialog)
  local layout = dia:GetLayout();
  layout:PushH()
  layout:AddChild(LM.GUI.StaticText("Directory:"))
  self.directory = LM.GUI.TextControl(200, file_path)
  layout:AddChild(self.directory)
  layout:Pop()
  layout:PushH()
  layout:AddChild(LM.GUI.StaticText("Project:"))
  self.project = LM.GUI.TextControl(200, project_name)
  layout:AddChild(self.project)
  layout:Pop()
  local popup = LM.GUI.PopupMenu(150, false)  
  layout:AddChild(popup)
  self.layer_menu = LM.GUI.Menu("Merge Layers")
  popup:SetMenu(self.layer_menu)
  return dia
end

function dialog:OnOK()
	file_path = dialog.directory:Value()
  project_name = dialog.project:Value()
  --print("Yeah!")
end

function bla(moho)
  local cge = require("cge")
  count = 0
  local doc = moho.document
  view_width = doc:Width()
  view_height = doc:Height()
  cam = doc.fCameraTrack:GetValue(0)
  print("image dimensions:"..view_width.."x"..view_height)
  
  local dia = dialog:new()
  
  count = 1
  traverseLayers(moho, doc, buildMenu)
  --dia.HandleMessage = handleMessage
  
  if dia:DoModal() ~= LM.GUI.MSG_OK then
    return
  end
  
  local groupname
  for i=1, dialog.layer_menu:CountItems() do
    local checked = dialog.layer_menu:IsChecked(dialog.layers[i])
    --print (checked and "True" or "Not")
    if not checked then
      groupname = layers[i]:Name()
      layer_groups[groupname] = {layers[i]}
    else
      layer_groups[groupname][#layer_groups[groupname]+1] = layers[i]
    end
  end
  
  cge.io.removeDir(file_path.."gfx")
  sleep(0.1)
  cge.io.createDir(file_path.."gfx")
  list_file = io.open(file_path.."list.gfx", "w")
  objects_file = io.open(file_path.."room.dat", "w")
  objects_file:write("//Room "..project_name.."\n")
  objects_file:write(view_width.."\n")
  objects_file:write(view_height.."\n")
  objects_file:write("0\n0\n5\n10\n3\n\n\n")
  
  count = 1
  traverseLayers(moho, doc, makeInvisble)
  count = 1
  --traverseLayers(moho, doc, drawLayer)
  for k,v in pairs(layer_groups) do
    print ("group "..k)
    drawLayers(k, moho, v)
  end
  count = 1
  traverseLayers(moho, doc, restoreVisibility)
  
  doc:SetShape(view_width, view_height)
  
  cge.io.packFiles(file_path.."gfx.roo", pictures)
  
  list_file:close()
  objects_file:close()
end