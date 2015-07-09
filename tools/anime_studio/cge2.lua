--settings
file_path = "C:\\tmp\\"
project_name = "test"

count = 0
view_width = 0
view_height = 0
cam = 0
visibility = {}
pictures = {}

dialog = {}
layer_menu = nil

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

function drawLayer(moho, layer)
  print("Layer: "..layer:Name())
  local vis = layer.fVisibility
  vis:SetValue(0, true)
  local bbox = layer:Bounds(0)
  --print(bbox.fMin.x.."/"..bbox.fMin.y.."/"..bbox.fMin.z)
  --print(bbox.fMax.x.."/"..bbox.fMax.y.."/"..bbox.fMax.z)
  local pos = moho.document.fCameraTrack:GetValue(0)
  local layerpos = layer.fTranslation:GetValue(0)
  local layerscale = layer.fScale:GetValue(0)
  --print(pos.x.."/"..pos.y.."/"..pos.z)
  pos.x = ((bbox.fMin.x+bbox.fMax.x)/2)*layerscale.x+layerpos.x--/layerscale.x--/pos.z
  pos.y = ((bbox.fMin.y+bbox.fMax.y)/2)*layerscale.y+layerpos.y--*layerscale.y
  local width = moho:DocToPixel((bbox.fMax.x-bbox.fMin.x)/pos.z*layerscale.x)
  local height = moho:DocToPixel((bbox.fMax.y-bbox.fMin.y)/pos.z*layerscale.y)
  width = math.ceil(width)
  height = math.ceil(height)
  pos.z = cam.z*(bbox.fMax.y-bbox.fMin.y)/2--cam.z*(view_width/width)
  --print(pos.x.."/"..pos.y.."/"..pos.z.."  ->"..width..";"..height)
  moho.document:SetShape(width, height)
  --print("wxh:"..moho.document:Width().." "..moho.document:Height())
  --position camera
  moho.document.fCameraTrack:SetValue(0, pos)
  
  --moho:SetSelLayer(layer)
  --moho.view:ResetView(1)
  --if count < 20 then
  local filename = file_path.."gfx/"..project_name.."_"..layer:Name()..".png"
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
  vis:SetValue(0, false)
end

function buildMenu(moho, layer)
  dialog.layers[count] = MOHO.MSG_BASE + count
  layer_menu:AddItem(layer:Name(), 0, dialog.layers[count])
  if (count == 1) then
    layer_menu:SetChecked(dialog.layers[count], true)
  end
  count = count + 1
end

function handleMessage(dia, message)
  local check = not layer_menu:IsChecked(message)
  layer_menu:SetChecked(message, check)
end

function bla(moho)
  local cge = require("cge")
  count = 0
  local doc = moho.document
  view_width = doc:Width()
  view_height = doc:Height()
  cam = doc.fCameraTrack:GetValue(0)
  print("image dimensions:"..view_width.."x"..view_height)
  
  dialog.layers = {}
  local dia = LM.GUI.SimpleDialog("Export Settings", dialog)
  local popup = LM.GUI.PopupMenu(100, false)  
  dia:GetLayout():AddChild(popup)
  layer_menu = LM.GUI.Menu("Layers")
  popup:SetMenu(layer_menu)
  
  count = 1
  traverseLayers(moho, doc, buildMenu)
  dia.HandleMessage = handleMessage
  
  if dia:DoModal() ~= LM.GUI.MSG_OK then
    return
  end
  
  cge.io.removeDir(file_path.."gfx")
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
  traverseLayers(moho, doc, drawLayer)
  count = 1
  traverseLayers(moho, doc, restoreVisibility)
  
  doc:SetShape(view_width, view_height)
  
  cge.io.packFiles(file_path.."gfx.roo", pictures)
  
  list_file:close()
  objects_file:close()
end