#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <image/image.h>
#include <image/loader.h>
#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <math/matrix.h>
#include <renderer/drawcontext.h>
#include <renderer/nulldevice.h>
#include <renderer/renderdevice.h>
#include <renderer/spritebatch.h>
#include <renderer/textureatlas.h>

#include "testenv.h"
#include "testwindow.h"

using namespace CGE;
using namespace CGE::Gfx;

// SpriteBatch and TextureAtlas. The batching is checked on the Null device,
// which counts draw calls and instances; the GL tests draw into render
// targets on a hidden window and read the pixels back. The atlases are
// tests/data/atlas/ (make_fixtures.py): an 8x4 page of 2x2 blocks.

namespace{

//! a Null device in a frame and a pass
class SpriteBatchTest : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("Null");
    ASSERT_TRUE(device_->init(NativeWindow(), DeviceDesc()));
    null_ = static_cast<NullDevice*>(device_.get());
    ctx_ = std::make_unique<DrawContext>(*device_);
    TextureDesc desc;
    desc.width = desc.height = 4;
    a_ = device_->createTexture(desc);
    b_ = device_->createTexture(desc);
  }
  void TearDown() override{
    a_.reset();
    b_.reset();
    ctx_.reset();
  }
  void begin(){
    ASSERT_TRUE(device_->beginFrame());
    ASSERT_TRUE(device_->beginPass(PassDesc()));
  }
  void end(){
    device_->endPass();
    device_->endFrame();
  }

  std::unique_ptr<RenderDevice> device_;
  NullDevice* null_ = nullptr;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<Texture> a_;
  std::unique_ptr<Texture> b_;
};

}

TEST_F(SpriteBatchTest, SpritesOfOneTextureAreOneDraw){
  SpriteBatch batch(*ctx_);
  begin();
  for (int i = 0; i < 100; ++i)
    batch.draw(a_.get(), (float)i, 0, 1, 1);
  EXPECT_EQ(batch.getPending(), 100);
  EXPECT_EQ(null_->getDrawCount(), 0);
  for (int i = 0; i < 50; ++i)
    batch.draw(b_.get(), (float)i, 1, 1, 1);
  //the change of texture drew the first ones
  EXPECT_EQ(null_->getDrawCount(), 1);
  EXPECT_EQ(null_->getInstanceCount(), 100);
  batch.flush();
  EXPECT_EQ(batch.getPending(), 0);
  EXPECT_EQ(null_->getDrawCount(), 2);
  EXPECT_EQ(null_->getInstanceCount(), 150);
  SpriteBatch::Stats stats = batch.getStats();
  EXPECT_EQ(stats.draws, 2);
  EXPECT_EQ(stats.sprites, 150);
  //nothing to draw
  batch.flush();
  EXPECT_EQ(null_->getDrawCount(), 2);
  end();
}

TEST_F(SpriteBatchTest, BlendModeAndSamplerChangesStartANewDraw){
  SpriteBatch batch(*ctx_);
  std::unique_ptr<Sampler> sampler = device_->createSampler(SamplerDesc());
  begin();
  EXPECT_EQ(batch.getBlend(), BlendMode::Alpha);
  batch.draw(a_.get(), 0, 0, 1, 1);
  batch.setBlend(BlendMode::Additive);
  batch.draw(a_.get(), 0, 0, 1, 1);
  batch.draw(a_.get(), 0, 0, 1, 1);
  batch.setSampler(sampler.get());
  batch.draw(a_.get(), 0, 0, 1, 1);
  //setting the same again doesn't
  batch.setSampler(sampler.get());
  batch.draw(a_.get(), 0, 0, 1, 1);
  batch.flush();
  EXPECT_EQ(null_->getDrawCount(), 3);
  EXPECT_EQ(null_->getInstanceCount(), 5);
  //the context's own state is kept
  EXPECT_EQ(ctx_->getBlend(), BlendMode::Opaque);
  EXPECT_EQ(ctx_->getTexture(), nullptr);
  end();
}

TEST_F(SpriteBatchTest, AFullBatchIsDrawn){
  SpriteBatch batch(*ctx_, 10);
  begin();
  for (int i = 0; i < 25; ++i)
    batch.draw(nullptr, 0, 0, 1, 1);
  batch.flush();
  EXPECT_EQ(null_->getDrawCount(), 3);
  EXPECT_EQ(null_->getInstanceCount(), 25);
  end();
}

TEST_F(SpriteBatchTest, SpritesNotFlushedInTheirFrameAreDropped){
  SpriteBatch batch(*ctx_);
  begin();
  batch.draw(a_.get(), 0, 0, 1, 1);
  end();
  begin();
  batch.draw(a_.get(), 0, 0, 1, 1);
  EXPECT_EQ(batch.getPending(), 1);
  batch.flush();
  EXPECT_EQ(null_->getInstanceCount(), 1);
  end();
  //and outside a pass, flush drops them
  ASSERT_TRUE(device_->beginFrame());
  batch.draw(a_.get(), 0, 0, 1, 1);
  batch.flush();
  EXPECT_EQ(batch.getPending(), 0);
  EXPECT_EQ(null_->getInstanceCount(), 1);
  EXPECT_EQ(batch.getStats().sprites, 0);
  device_->endFrame();
}

TEST_F(SpriteBatchTest, AtlasRegionsReadFromJsonHash){
  TextureAtlas atlas(*device_);
  ASSERT_TRUE(atlas.load("atlas/hash.json"));
  EXPECT_EQ(atlas.getPageCount(), 1);
  EXPECT_FLOAT_EQ(atlas.getScale(), 1);
  ASSERT_EQ(atlas.getRegions().size(), 6u);
  EXPECT_EQ(atlas.getPage(0)->getDesc().width, 8);
  EXPECT_EQ(atlas.getPage(0)->getDesc().height, 4);

  const AtlasRegion* red = atlas.find("red");
  ASSERT_NE(red, nullptr);
  EXPECT_EQ(red->texture, atlas.getPage(0));
  EXPECT_FLOAT_EQ(red->rect.u0, 0);
  EXPECT_FLOAT_EQ(red->rect.v0, 0);
  EXPECT_FLOAT_EQ(red->rect.u1, 0.25f);
  EXPECT_FLOAT_EQ(red->rect.v1, 0.5f);
  EXPECT_FLOAT_EQ(red->width, 2);
  EXPECT_FLOAT_EQ(red->trimWidth, 2);
  EXPECT_FLOAT_EQ(red->pivotX, 0.5f);

  const AtlasRegion* green = atlas.find("green");
  ASSERT_NE(green, nullptr);
  EXPECT_FLOAT_EQ(green->rect.u0, 0.25f);
  EXPECT_FLOAT_EQ(green->width, 4);
  EXPECT_FLOAT_EQ(green->height, 4);
  EXPECT_FLOAT_EQ(green->trimX, 1);
  EXPECT_FLOAT_EQ(green->trimY, 2);
  EXPECT_FLOAT_EQ(green->trimWidth, 2);
  EXPECT_FLOAT_EQ(green->trimHeight, 2);
  EXPECT_FLOAT_EQ(green->pivotX, 0.25f);
  EXPECT_FLOAT_EQ(green->pivotY, 1);

  const AtlasRegion* blue = atlas.find("blue_2");
  ASSERT_NE(blue, nullptr);
  EXPECT_FLOAT_EQ(blue->rect.u0, 0.25f);
  EXPECT_FLOAT_EQ(blue->rect.v0, 0.5f);
  EXPECT_EQ(atlas.find("red.png"), nullptr);
  EXPECT_EQ(atlas.find("missing"), nullptr);
}

TEST_F(SpriteBatchTest, AtlasJsonArrayIsTheSame){
  TextureAtlas hash(*device_);
  TextureAtlas array(*device_);
  ASSERT_TRUE(hash.load("atlas/hash.json"));
  ASSERT_TRUE(array.load("atlas/array.json"));
  ASSERT_EQ(array.getRegions().size(), hash.getRegions().size());
  for (size_t i = 0; i < hash.getRegions().size(); ++i){
    const AtlasRegion& h = hash.getRegions()[i];
    const AtlasRegion& a = array.getRegions()[i];
    EXPECT_EQ(a.name, h.name);
    EXPECT_FLOAT_EQ(a.rect.u0, h.rect.u0);
    EXPECT_FLOAT_EQ(a.rect.v1, h.rect.v1);
    EXPECT_FLOAT_EQ(a.width, h.width);
    EXPECT_FLOAT_EQ(a.trimY, h.trimY);
  }
}

TEST_F(SpriteBatchTest, AtlasPagesAndScale){
  TextureAtlas atlas(*device_);
  ASSERT_TRUE(atlas.load("atlas/pages.json"));
  EXPECT_EQ(atlas.getPageCount(), 2);
  //"2" as a string and as a number
  EXPECT_FLOAT_EQ(atlas.getScale(), 2);
  const AtlasRegion* red = atlas.find("red");
  const AtlasRegion* blue = atlas.find("blue_10");
  ASSERT_NE(red, nullptr);
  ASSERT_NE(blue, nullptr);
  EXPECT_EQ(red->page, 0);
  EXPECT_EQ(blue->page, 1);
  EXPECT_EQ(blue->texture, atlas.getPage(1));
  //sizes in units, not in the 2x pixels
  EXPECT_FLOAT_EQ(red->width, 1);
  EXPECT_FLOAT_EQ(blue->height, 1);
  const AtlasRegion* green = atlas.find("green");
  ASSERT_NE(green, nullptr);
  EXPECT_FLOAT_EQ(green->width, 2);
  EXPECT_FLOAT_EQ(green->trimX, 0.5f);
  //the texture coordinates stay those of the page
  EXPECT_FLOAT_EQ(green->rect.u1, 0.5f);
}

TEST_F(SpriteBatchTest, AtlasSequencesAreInNumericOrder){
  TextureAtlas atlas(*device_);
  ASSERT_TRUE(atlas.load("atlas/hash.json"));
  std::vector<const AtlasRegion*> blue = atlas.findSequence("blue_");
  ASSERT_EQ(blue.size(), 4u);
  EXPECT_EQ(blue[0]->name, "blue_0");
  EXPECT_EQ(blue[1]->name, "blue_1");
  EXPECT_EQ(blue[2]->name, "blue_2");
  EXPECT_EQ(blue[3]->name, "blue_10");
  EXPECT_TRUE(atlas.findSequence("red_").empty());
  EXPECT_TRUE(atlas.findSequence("re").empty());
}

TEST_F(SpriteBatchTest, AtlasErrors){
  TextureAtlas atlas(*device_);
  EXPECT_FALSE(atlas.load("atlas/missing.json"));
  EXPECT_EQ(atlas.getPageCount(), 0);
  EXPECT_FALSE(atlas.loadFromText("{\"frames\": ", "atlas"));
  //a missing page image
  EXPECT_FALSE(atlas.loadFromText(R"({"frames": {}, "meta": {"image": "missing.png"}})", "atlas"));
  EXPECT_EQ(atlas.getPageCount(), 0);
  //rotated frames are skipped, the others kept
  EXPECT_FALSE(atlas.loadFromText(R"({"frames": {
    "a": {"frame": {"x": 0, "y": 0, "w": 2, "h": 2}},
    "b": {"frame": {"x": 2, "y": 0, "w": 2, "h": 2}, "rotated": true}},
    "meta": {"image": "page.png"}})", "atlas"));
  EXPECT_NE(atlas.find("a"), nullptr);
  EXPECT_EQ(atlas.find("b"), nullptr);
  //without trimming information, the frame is the whole image
  const AtlasRegion* a = atlas.find("a");
  ASSERT_NE(a, nullptr);
  EXPECT_FLOAT_EQ(a->width, 2);
  EXPECT_FLOAT_EQ(a->trimX, 0);
  atlas.clear();
  EXPECT_TRUE(atlas.getRegions().empty());
  EXPECT_EQ(atlas.find("a"), nullptr);
}

TEST_F(SpriteBatchTest, AtlasRegionSpriteHasTheSizeAndPivot){
  AtlasRegion region;
  region.width = 10;
  region.height = 20;
  region.pivotX = 0.25f;
  region.pivotY = 1;
  Sprite sprite = region.sprite(3, 4);
  EXPECT_FLOAT_EQ(sprite.x, 3);
  EXPECT_FLOAT_EQ(sprite.y, 4);
  EXPECT_FLOAT_EQ(sprite.width, 10);
  EXPECT_FLOAT_EQ(sprite.height, 20);
  EXPECT_FLOAT_EQ(sprite.originX, 0.25f);
  EXPECT_FLOAT_EQ(sprite.originY, 1);
}

#if defined(CGE_TEST_WINDOW)

namespace{

using TestEnv::TestWindow;

struct Pixel{
  uint8_t r, g, b, a;
};

::testing::AssertionResult isNear(const Pixel& p, int r, int g, int b, int tolerance = 2){
  if (abs(p.r-r) <= tolerance && abs(p.g-g) <= tolerance && abs(p.b-b) <= tolerance)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "pixel (" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b
    << ") instead of (" << r << ", " << g << ", " << b << ")";
}

//! collects the errors and warnings that are traced
class ProblemCollector : public CGE::TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    if (level <= TRACE_WARNING)
      problems.push_back(message);
  }
  std::vector<std::string> problems;
};

class Sprites : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("OpenGL");
    ASSERT_NE(device_, nullptr);
    DeviceDesc desc;
    desc.debug = true;
    if (!window_.create(*device_, desc))
      GTEST_SKIP() << "no window";
    if (!device_->init(window_.native(), desc))
      GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
    ctx_ = std::make_unique<DrawContext>(*device_);
    batch_ = std::make_unique<SpriteBatch>(*ctx_);
    SamplerDesc nearest;
    nearest.minFilter = nearest.magFilter = Filter::Nearest;
    nearest.addressU = nearest.addressV = AddressMode::ClampToEdge;
    nearest_ = device_->createSampler(nearest);
    batch_->setSampler(nearest_.get());
    CGE::TraceManager::instance()->setTraceOutputter(&collector_);
    collector_.problems.clear();
    collecting_ = true;
  }

  void TearDown() override{
    nearest_.reset();
    batch_.reset();
    ctx_.reset();
    if (collecting_){
      CGE::TraceManager::instance()->removeTraceOutputter(&collector_);
      for (const std::string& problem : collector_.problems)
        ADD_FAILURE() << "traced: " << problem;
    }
  }

  std::unique_ptr<Gfx::RenderTarget> target(int width, int height){
    RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.depthFormat = Format::Undefined;
    return device_->createRenderTarget(desc);
  }

  //! a frame and a black pass into target, with a projection in pixels, y down
  void begin(Gfx::RenderTarget* target){
    ASSERT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target;
    pass.clearColor = {0, 0, 0, 1};
    ASSERT_TRUE(device_->beginPass(pass));
    const RenderTargetDesc& desc = target->getDesc();
    ctx_->setProjection(device_->ortho(0, (float)desc.width, (float)desc.height, 0, -10, 10));
    ctx_->setView(Matrix(Matrix::Identity));
  }

  void end(){
    batch_->flush();
    device_->endPass();
    device_->endFrame();
  }

  //! the pixels, rows from the top down (y down, like the sprites)
  std::vector<Pixel> read(Gfx::RenderTarget* target){
    const RenderTargetDesc& desc = target->getDesc();
    std::vector<Pixel> bottomUp(desc.width*desc.height);
    EXPECT_TRUE(device_->readPixels(target, 0, {0, 0, desc.width, desc.height}, bottomUp.data()));
    std::vector<Pixel> pixels(bottomUp.size());
    for (int y = 0; y < desc.height; ++y){
      for (int x = 0; x < desc.width; ++x)
        pixels[y*desc.width+x] = bottomUp[(desc.height-1-y)*desc.width+x];
    }
    return pixels;
  }

  //declared first, so the device goes before the window
  TestWindow window_;
  std::unique_ptr<RenderDevice> device_;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<SpriteBatch> batch_;
  std::unique_ptr<Sampler> nearest_;
  ProblemCollector collector_;
  bool collecting_ = false;
};

}

TEST_F(Sprites, RectanglesTakeTheirColours){
  auto rt = target(8, 8);
  begin(rt.get());
  batch_->draw(nullptr, 1, 1, 2, 3, TexRect(), Color(1, 0, 0, 1));
  batch_->draw(nullptr, 5, 0, 3, 1, TexRect(), Color(0, 0, 1, 1));
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 8; ++y){
    for (int x = 0; x < 8; ++x){
      const Pixel& p = pixels[y*8+x];
      if (x >= 1 && x < 3 && y >= 1 && y < 4)
        EXPECT_TRUE(isNear(p, 255, 0, 0)) << x << ", " << y;
      else if (x >= 5 && y == 0)
        EXPECT_TRUE(isNear(p, 0, 0, 255)) << x << ", " << y;
      else
        EXPECT_TRUE(isNear(p, 0, 0, 0)) << x << ", " << y;
    }
  }
}

TEST_F(Sprites, TexturesAreUprightAndFlip){
  //image.png, 3x2: red green blue / white black brown
  std::unique_ptr<Image> image(ImageLoader::load(TestEnv::dataFile("image.png").c_str()));
  ASSERT_NE(image, nullptr);
  image->convertFormat(4);
  TextureDesc desc;
  desc.width = 3;
  desc.height = 2;
  std::unique_ptr<Texture> tex = device_->createTexture(desc, image->getData());
  auto rt = target(6, 2);
  begin(rt.get());
  batch_->draw(tex.get(), 0, 0, 3, 2);
  Sprite flipped;
  flipped.x = 3;
  flipped.width = 3;
  flipped.height = 2;
  flipped.flipX = true;
  flipped.flipY = true;
  batch_->draw(tex.get(), flipped);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  const int picture[2][3][3] = {
    {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}},
    {{255, 255, 255}, {0, 0, 0}, {128, 64, 32}},
  };
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const int* upright = picture[y][x];
      EXPECT_TRUE(isNear(pixels[y*6+x], upright[0], upright[1], upright[2])) << x << ", " << y;
      const int* mirrored = picture[1-y][2-x];
      EXPECT_TRUE(isNear(pixels[y*6+3+x], mirrored[0], mirrored[1], mirrored[2])) << x << ", " << y;
    }
  }
  tex.reset();
}

TEST_F(Sprites, SpritesRotateAndScaleAroundTheirOrigin){
  auto rt = target(8, 8);
  begin(rt.get());
  //a 4x2 bar, centred at (4, 4), turned by 90 degrees: 2 wide, 4 high
  Sprite bar;
  bar.x = 4;
  bar.y = 4;
  bar.width = 2;
  bar.height = 1;
  bar.scaleX = 2;
  bar.scaleY = 2;
  bar.originX = 0.5f;
  bar.originY = 0.5f;
  bar.rotation = 90;
  bar.color = Color(0, 1, 0, 1);
  batch_->draw(nullptr, bar);
  //a matrix: the unit quad scaled to 2x1 at (0, 7)
  Matrix m = Matrix(Matrix::Translation, Vec3f(0, 7, 0))*Matrix(Matrix::Scale, Vec3f(2, 1, 1));
  batch_->draw(nullptr, m, TexRect(), Color(1, 0, 0, 1));
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 8; ++y){
    for (int x = 0; x < 8; ++x){
      const Pixel& p = pixels[y*8+x];
      if (x >= 3 && x < 5 && y >= 2 && y < 6)
        EXPECT_TRUE(isNear(p, 0, 255, 0)) << x << ", " << y;
      else if (x < 2 && y == 7)
        EXPECT_TRUE(isNear(p, 255, 0, 0)) << x << ", " << y;
      else
        EXPECT_TRUE(isNear(p, 0, 0, 0)) << x << ", " << y;
    }
  }
}

TEST_F(Sprites, EveryFlushKeepsItsInstances){
  //eight draws in one pass: each must keep its own instance data
  auto rt = target(8, 1);
  SpriteBatch small(*ctx_, 3);
  begin(rt.get());
  for (int i = 0; i < 8; ++i){
    batch_->draw(nullptr, (float)i, 0, 1, 1, TexRect(), Color(i/7.0f, 1-i/7.0f, (float)(i%2), 1));
    batch_->flush();
  }
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int i = 0; i < 8; ++i)
    EXPECT_TRUE(isNear(pixels[i], (int)(i/7.0f*255+0.5f), (int)((1-i/7.0f)*255+0.5f), (i%2)*255)) << "draw " << i;

  //more sprites than fit into one draw, and more than one instance buffer
  begin(rt.get());
  for (int i = 0; i < 8; ++i)
    small.draw(nullptr, (float)i, 0, 1, 1, TexRect(), Color(1, 1, 1, 1));
  small.flush();
  EXPECT_EQ(small.getStats().draws, 3);
  end();
  pixels = read(rt.get());
  for (int i = 0; i < 8; ++i)
    EXPECT_TRUE(isNear(pixels[i], 255, 255, 255)) << "sprite " << i;
}

TEST_F(Sprites, BlendModes){
  auto rt = target(2, 1);
  begin(rt.get());
  batch_->setBlend(BlendMode::Opaque);
  batch_->draw(nullptr, 0, 0, 2, 1, TexRect(), Color(0.5f, 0, 0, 1));
  batch_->setBlend(BlendMode::Additive);
  batch_->draw(nullptr, 0, 0, 1, 1, TexRect(), Color(0, 1, 0, 1));
  batch_->setBlend(BlendMode::Alpha);
  batch_->draw(nullptr, 1, 0, 1, 1, TexRect(), Color(0, 0, 1, 0.5f));
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[0], 128, 255, 0));
  EXPECT_TRUE(isNear(pixels[1], 64, 0, 128));
}

TEST_F(Sprites, AtlasRegionsAreDrawnWhereTheyWereBeforeTrimming){
  TextureAtlas atlas(*device_);
  ASSERT_TRUE(atlas.load("atlas/hash.json"));
  auto rt = target(8, 4);
  begin(rt.get());
  //green: 2x2 at (1, 2) in a 4x4 image, pivot (0.25, 1): the image from (0, 0) to (4, 4)
  batch_->draw(*atlas.find("green"), 1, 4);
  //red: untrimmed 2x2 with the pivot in the centre
  batch_->draw(*atlas.find("red"), 6, 2);
  //blue_0 mirrored: dark blue 2x2 from (3, 0) (the region is all one colour)
  Sprite blue = atlas.find("blue_0")->sprite(4, 1);
  blue.flipX = true;
  batch_->draw(*atlas.find("blue_0"), blue);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 4; ++y){
    for (int x = 0; x < 8; ++x){
      const Pixel& p = pixels[y*8+x];
      if (x >= 1 && x < 3 && y >= 2)
        EXPECT_TRUE(isNear(p, 0, 255, 0)) << x << ", " << y;
      else if (x >= 5 && x < 7 && y >= 1 && y < 3)
        EXPECT_TRUE(isNear(p, 255, 0, 0)) << x << ", " << y;
      else if (x >= 3 && x < 5 && y < 2)
        EXPECT_TRUE(isNear(p, 0, 0, 64)) << x << ", " << y;
      else
        EXPECT_TRUE(isNear(p, 0, 0, 0)) << x << ", " << y;
    }
  }
}

#endif
