#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CGE{
namespace Gfx{
class Texture;
}
}

//! TDClient's textures on the render device, by category and index
class TextureManager{
  public:
    typedef std::vector<std::unique_ptr<CGE::Gfx::Texture>> Textures;
    static void init() { texMan_ = new TextureManager(); atexit(release); }
    static TextureManager* instance() {return texMan_;}
    //! deletes the textures; before the render device goes
    static void release() { delete texMan_; texMan_ = NULL; }
    Textures floorTex;
    Textures wallTex;
    Textures heroTex;
    Textures monsterTex;
    Textures furnitureTex;
    Textures overlayTex;
    Textures otherTex;
    //! the icon of an item, a piece of equipment or a spell by its name (textures/items); nullptr if it has none
    CGE::Gfx::Texture* getItemIcon(const std::string& name) const;
    //! draws an item's icon into the inventory's or the shop's slot whose name label starts at
    //! (labelX, labelY) (virtual pixels, y up), centred over the label
    void drawItemIcon(const std::string& name, int labelX, int labelY) const;
  private:
    std::map<std::string, std::unique_ptr<CGE::Gfx::Texture>> itemIcons_;
    TextureManager();
    ~TextureManager();
    void loadTextures();
    //! loads the textures a .dat file lists (a header, the count, then name and index per texture)
    static void loadFromDat(const std::string& path, const std::string& filename, Textures& textures);
    static TextureManager* texMan_;
};

#endif
