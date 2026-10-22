#ifndef ADV_GRAPHICS_H
#define ADV_GRAPHICS_H

#include <memory>
#include <vector>

#include <renderer/gfxtypes.h>
#include <math/vector.h>

#include "Vector.h"

namespace CGE{
class Image;
class MatrixStack;
namespace Gfx{
class Buffer;
class DrawContext;
class RenderDevice;
class RenderTarget;
class Sampler;
class Shader;
class Texture;
}
}

namespace adv{

//! The Adventure's drawing on the engine's render device
/*! Keeps the state the Adventure's drawing code used to set on the old
    renderer (model matrices, colour, blending, texture, clear colour,
    render targets) and draws through the engine's Gfx::DrawContext.
    Everything is drawn in the game's resolution with y pointing down.
    Render targets become passes: pushTarget ends the pass being drawn and
    begins one into the target, popTarget continues the previous pass.
    A frame is drawn into a render target of the window's size, the screen,
    which endFrame copies into the window. Unlike the window, it keeps what
    it shows from one frame to the next. */
class Graphics{
public:
  //! the uniform buffer slot of the Adventure's blocks (ADV_Blit, ADV_Effect)
  static const int SLOT_ADVENTURE = 4;

  enum Blend{
    //! src*alpha + dst*(1-alpha)
    BLEND_ALPHA,
    //! src*alpha + dst
    BLEND_ADDITIVE,
    //! src + dst*(1-alpha)
    BLEND_PREMULT_ALPHA,
    //! src*dst
    BLEND_MULTIPLY,
  };

  //! a vertex of the quads and shapes: position and texture coordinates
  struct Vertex{
    float x, y, z;
    float u, v;
  };

  Graphics();
  //! deletes the resources it made; before the engine's device goes
  ~Graphics();

  CGE::Gfx::RenderDevice& getDevice();
  CGE::Gfx::DrawContext& getContext();
  //! the layout of Vertex: position (location 0), texture coordinates (location 3)
  static const CGE::Gfx::VertexLayout& getVertexLayout();
  //! the unit quad from (0, 0) to (1, 1) as a triangle strip of 4 vertices
  static void getQuad(Vertex quad[4]);

  // --- frames and render targets ---

  //! begins the frame's pass into the screen, which still shows the last frame
  /*! Resets the model matrix. A new screen (at the start, or when the
      window's size changed) is cleared with the clear colour. */
  bool beginFrame();
  //! ends the frame and copies the screen into the window
  void endFrame();
  //! draws into a render target of the size of the game's resolution or less
  /*! The target's area from the bottom left corner of size is used, and
      the game's resolution is mapped onto it. Nests. */
  void pushTarget(CGE::Gfx::RenderTarget* target, const Vec2i& size);
  //! goes back to the target (or the screen) drawn into before pushTarget
  void popTarget();
  //! where the screen shows the game, moved by offset (in the game's pixels, y up)
  /*! moving: clear only clears the moved area and the rest of the screen
      keeps what it showed (the camera viewport moving between rooms). */
  void setWindowOffset(const Vec2f& offset, bool moving);
  void setClearColor(float r, float g, float b, float a);
  void setClearDepth(float depth);
  //! clears the current target's colour and depth
  void clear();

  // --- drawing state ---

  //! the model matrix stack
  CGE::MatrixStack& model();
  void setColor(float r, float g, float b, float a);
  void setBlend(Blend blend);
  //! the texture of the next draws, or nullptr for untextured draws
  /*! The texture coordinates are scaled by scale, then offset is added. */
  void setTexture(CGE::Gfx::Texture* texture, const Vec2f& scale = Vec2f(1, 1), const Vec2f& offset = Vec2f(0, 0));
  //! depth testing and writing (off by default)
  void setDepth(bool test, bool write, CGE::Gfx::CompareFunc compare = CGE::Gfx::CompareFunc::LessEqual);
  void setColorWrite(bool write);
  //! the sampler of the Adventure's textures: linear, repeating
  CGE::Gfx::Sampler* getSampler();
  //! the unit quad of getQuad as a vertex buffer
  CGE::Gfx::Buffer* getQuadBuffer();

  // --- drawing ---

  //! the unit quad from (0, 0) to (1, 1) in the model matrix
  void drawQuad();
  //! the outline of the unit quad
  void drawQuadLines();
  //! count Vertex from the buffer, starting at first
  void draw(CGE::Gfx::Buffer* vertices, CGE::Gfx::PrimitiveType primitive, int count, int first = 0);
  //! sets an own shader with the engine's blocks, the texture in slot 0 and
  //! uniforms in the ADV_ block (SLOT_ADVENTURE); then bind more and draw
  bool prepare(CGE::Gfx::Shader* shader, const CGE::Gfx::VertexLayout& layout, CGE::Gfx::PrimitiveType primitive,
    const void* uniforms, size_t size);

  // --- resources ---

  //! a vertex buffer of count Vertex, filled with vertices if it isn't nullptr
  std::unique_ptr<CGE::Gfx::Buffer> createVertices(int count, const Vertex* vertices, CGE::Gfx::Usage usage);
  //! a shader from two files of the Adventure's shaders ("adventure/...")
  /*! It gets the engine's bindings, ADV_Blit or ADV_Effect at
      SLOT_ADVENTURE, the sampler tex in slot 0 and the further samplers in
      slots 1, 2, ... */
  std::unique_ptr<CGE::Gfx::Shader> createShader(const char* vertexFile, const char* fragmentFile,
    const std::vector<const char*>& samplers = std::vector<const char*>());
  //! deletes a shader made with createShader
  void destroyShader(std::unique_ptr<CGE::Gfx::Shader>& shader);
  //! an RGBA texture of width x height, filled with an image if it isn't nullptr
  /*! The image (at most the texture's size) goes into the top left corner,
      converted by toRGBA. */
  CGE::Gfx::Texture* createTexture(int width, int height, const CGE::Image* image = nullptr, const CGE::Image* alpha = nullptr);
  //! writes an image into the top left corner of a texture made by createTexture
  void updateTexture(CGE::Gfx::Texture* texture, const CGE::Image* image, const CGE::Image* alpha = nullptr);
  //! the pixels of an image as RGBA
  /*! Grey images become grey with full alpha. Images without alpha take it
      from the alpha image's first channel or, without one, are opaque
      except for the colour key magenta (255, 0, 255). */
  static std::vector<unsigned char> toRGBA(const CGE::Image* image, const CGE::Image* alpha = nullptr);

private:
  struct Target{
    CGE::Gfx::RenderTarget* target;
    Vec2i size;
    //! the screen, which shows the game as the window does
    bool screen;
  };
  //! begins a pass into the top of the target stack
  bool beginPass(CGE::Gfx::LoadAction load);
  //! sets the projection (and viewport) of the top of the target stack
  void setupTarget();
  bool prepareDraw(CGE::Gfx::PrimitiveType primitive);

  CGE::Gfx::DrawContext& mContext;
  CGE::Gfx::RenderDevice& mDevice;
  std::vector<Target> mTargets;
  bool mInPass;
  CGE::Gfx::ClearColor mClearColor;
  float mClearDepth;
  Vec2f mWindowOffset;
  bool mWindowMoving;
  CGE::Gfx::Texture* mTexture;
  CGE::Gfx::Sampler* mTextureSampler;
  float mTexTransform[4];
  std::unique_ptr<CGE::Gfx::Shader> mTextured;
  std::unique_ptr<CGE::Gfx::Shader> mUntextured;
  std::unique_ptr<CGE::Gfx::Buffer> mQuad;
  std::unique_ptr<CGE::Gfx::Buffer> mQuadLines;
  std::unique_ptr<CGE::Gfx::Sampler> mSampler;
  //! for copying the screen into the window
  std::unique_ptr<CGE::Gfx::Sampler> mNearest;
  std::unique_ptr<CGE::Gfx::RenderTarget> mScreen;
  Vec2i mScreenSize;
};

}

#endif
