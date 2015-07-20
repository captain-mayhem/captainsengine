#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

namespace Graphics{
class Texture;
}

using Graphics::Texture;

class TextureManager{
  public:
    static void init() {texMan_ = new TextureManager();}
    static TextureManager* instance() {return texMan_;}
    Texture** floorTex;
    Texture** wallTex;
    Texture** heroTex;
    Texture** monsterTex;
    Texture** furnitureTex;
    Texture** overlayTex;
    Texture** otherTex;
  private:
    TextureManager();
    ~TextureManager();
    void loadTextures();
    static TextureManager* texMan_;
    int floorNum_;
    int wallNum_;
    int heroNum_;
    int monsterNum_;
    int furnitureNum_;
    int overlayNum_;
    int otherNum_;
};

#endif

