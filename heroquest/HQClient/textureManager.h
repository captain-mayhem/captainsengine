#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

namespace CGE{
class Texture;
}

class TextureManager{
  public:
    static void init() {texMan_ = new TextureManager();}
    static TextureManager* instance() {return texMan_;}
    CGE::Texture** floorTex;
    CGE::Texture** wallTex;
    CGE::Texture** heroTex;
    CGE::Texture** monsterTex;
    CGE::Texture** furnitureTex;
    CGE::Texture** overlayTex;
    CGE::Texture** otherTex;
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

