#include <sstream>

#include "system/engine.h"
#include "platform/assets.h"
#include "io/Tracing.h"
#include "renderer/renderdevice.h"
#include "renderer/textureloader.h"
#include "renderer/drawcontext.h"
#include "renderer/forms.h"
#include "textureManager.h"

TR_CHANNEL(TD_Textures);

TextureManager* TextureManager::texMan_ = NULL;

TextureManager::TextureManager(){
  loadTextures();
}

TextureManager::~TextureManager(){
}

void TextureManager::loadTextures(){
  loadFromDat("textures/map/","ground.dat",floorTex);
  loadFromDat("textures/world/","wall.dat",wallTex);
  loadFromDat("textures/heros/","heros.dat",heroTex);
  loadFromDat("textures/monsters/","monsters.dat",monsterTex);
  loadFromDat("textures/furniture/","furniture.dat",furnitureTex);
  loadFromDat("textures/overlays/","overlays.dat",overlayTex);
  loadFromDat("textures/other/","other.dat",otherTex);
  //the item icons: icons.dat lists the names, a line each; the file is <name>.png,
  //or a second word on the line names another icon (an item that looks like it)
  TR_USE(TD_Textures);
  std::string text;
  if (!CGE::Assets::loadText("textures/items/icons.dat", text)){
    TR_WARN("Cannot load icons.dat");
    return;
  }
  CGE::Gfx::RenderDevice* device = CGE::Engine::instance()->getRenderDevice();
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)){
    std::istringstream words(line);
    std::string name, file;
    if (!(words >> name))
      continue;
    if (!(words >> file))
      file = name;
    itemIcons_[name] = CGE::Gfx::loadTexture(*device, "textures/items/"+file+".png");
  }
}

CGE::Gfx::Texture* TextureManager::getItemIcon(const std::string& name) const{
  auto it = itemIcons_.find(name);
  return it == itemIcons_.end() ? nullptr : it->second.get();
}

void TextureManager::drawItemIcon(const std::string& name, int labelX, int labelY) const{
  CGE::Gfx::Texture* icon = getItemIcon(name);
  if (!icon)
    return;
  //the slots are 90 wide, their labels start 15 left of them; the icon fills the slot
  //above the label (textures/other/inventory.jpg and shop.jpg, tools/make_ui_art.py)
  const float size = 88;
  CGE::Gfx::DrawContext& ctx = *CGE::Engine::instance()->getDrawContext();
  CGE::Gfx::BlendMode blend = ctx.getBlend();
  ctx.setBlend(CGE::Gfx::BlendMode::Alpha);
  ctx.setColor(1, 1, 1, 1);
  ctx.setTexture(icon);
  CGE::Engine::instance()->getForms()->drawQuad(ctx, CGE::Vec2f(labelX + 15 + 45 - size/2, labelY + 44.0f), CGE::Vec2f(size, size));
  ctx.setTexture(nullptr);
  ctx.setBlend(blend);
}

void TextureManager::loadFromDat(const std::string& path, const std::string& filename, Textures& textures){
  TR_USE(TD_Textures);
  std::string text;
  if (!CGE::Assets::loadText(path+filename, text)){
    TR_WARN("Cannot load %s", filename.c_str());
    return;
  }
  CGE::Gfx::RenderDevice* device = CGE::Engine::instance()->getRenderDevice();
  std::istringstream in(text);
  std::string name;
  int number = 0;
  in >> name;
  in >> number;
  textures.resize(number);
  for (int i = 0; i < number; i++){
    int idx = -1;
    in >> name;
    in >> idx;
    if (idx < 0 || idx >= number){
      TR_WARN("%s: index %d of %s out of range", filename.c_str(), idx, name.c_str());
      continue;
    }
    textures[idx] = CGE::Gfx::loadTexture(*device, path+name);
  }
}
