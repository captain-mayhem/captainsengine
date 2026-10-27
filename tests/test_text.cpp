#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <math/matrix.h>
#include <platform/systemfonts.h>
#include <renderer/drawcontext.h>
#include <renderer/font.h>
#include <renderer/nulldevice.h>
#include <renderer/renderdevice.h>
#include <renderer/spritebatch.h>

#include "testenv.h"
#include "testwindow.h"

using namespace CGE;
using namespace CGE::Gfx;

// Text: UTF-8 decoding, CGE::Font (FreeType glyphs through a SpriteBatch) and
// SystemFonts. The layout and the batching are checked on the Null device;
// the GL tests draw into render targets on a hidden window and read the
// pixels back. The fonts are the engine's (Engine/fonts). CGE::Font is
// written out because of X11's Font.

namespace{

std::string engineFont(const char* name){
  return std::string(CGE_TEST_FONTS_DIR) + "/" + name;
}

std::vector<char32_t> decode(const std::string& text){
  std::vector<char32_t> result;
  size_t pos = 0;
  while (pos < text.size())
    result.push_back(nextCharacter(text, pos));
  return result;
}

}

TEST(Utf8, DecodesAllOfUnicode){
  //A, a umlaut, euro, a gem (beyond the BMP), Cyrillic Zhe
  EXPECT_EQ(decode("A\xC3\xA4\xE2\x82\xAC\xF0\x9F\x92\x8E\xD0\x96"),
    (std::vector<char32_t>{U'A', 0xE4, 0x20AC, 0x1F48E, 0x416}));
}

TEST(Utf8, TakesOtherBytesAsLatin1){
  //an ISO 8859-1 umlaut, a cut off sequence, an overlong one and a surrogate
  EXPECT_EQ(decode("\xE4x"), (std::vector<char32_t>{0xE4, U'x'}));
  EXPECT_EQ(decode("\xE2\x82"), (std::vector<char32_t>{0xE2, 0x82}));
  EXPECT_EQ(decode("\xE0\x80\x80"), (std::vector<char32_t>{0xE0, 0x80, 0x80}));
  EXPECT_EQ(decode("\xED\xA0\x80"), (std::vector<char32_t>{0xED, 0xA0, 0x80}));
}

namespace{

//! the engine's font on a Null device, in a frame and a pass
class Text : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("Null");
    ASSERT_TRUE(device_->init(NativeWindow(), DeviceDesc()));
    null_ = static_cast<NullDevice*>(device_.get());
    ctx_ = std::make_unique<DrawContext>(*device_);
    batch_ = std::make_unique<SpriteBatch>(*ctx_);
    font_ = CGE::Font::load(*device_, engineFont("DejaVuSans.ttf"));
    ASSERT_NE(font_, nullptr);
  }
  void TearDown() override{
    font_.reset();
    batch_.reset();
    ctx_.reset();
  }
  void begin(){
    ASSERT_TRUE(device_->beginFrame());
    ASSERT_TRUE(device_->beginPass(PassDesc()));
  }
  void end(){
    batch_->flush();
    device_->endPass();
    device_->endFrame();
  }

  std::unique_ptr<RenderDevice> device_;
  NullDevice* null_ = nullptr;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<SpriteBatch> batch_;
  std::unique_ptr<CGE::Font> font_;
};

}

TEST_F(Text, LoadsTheEngineFont){
  EXPECT_EQ(font_->getFamily(), "DejaVu Sans");
  EXPECT_EQ(font_->getDesc().size, 16);
  EXPECT_TRUE(font_->hasGlyph(U'A'));
  EXPECT_TRUE(font_->hasGlyph(0xE4));
  EXPECT_TRUE(font_->hasGlyph(0x20AC));
  EXPECT_TRUE(font_->hasGlyph(0x416));
  //no CJK in DejaVu Sans
  EXPECT_FALSE(font_->hasGlyph(0x4E00));
}

TEST_F(Text, LoadsNoOtherFiles){
  EXPECT_EQ(CGE::Font::load(*device_, std::vector<uint8>{1, 2, 3, 4}), nullptr);
  EXPECT_EQ(CGE::Font::load(*device_, engineFont("missing.ttf")), nullptr);
}

TEST_F(Text, MetricsScaleWithTheSize){
  EXPECT_GT(font_->getAscent(), 10);
  EXPECT_GT(font_->getDescent(), 2);
  EXPECT_GE(font_->getLineHeight(), font_->getAscent() + font_->getDescent() - 0.01f);
  EXPECT_FLOAT_EQ(font_->getAscent(32), 2*font_->getAscent());
  EXPECT_FLOAT_EQ(font_->getLineHeight(8), font_->getLineHeight()/2);
}

TEST_F(Text, MeasuresKernedLines){
  Vec2f a = font_->measure("A");
  Vec2f v = font_->measure("V");
  Vec2f av = font_->measure("AV");
  EXPECT_GT(a.x, 5);
  //the pair is kerned closer together
  EXPECT_LT(av.x, a.x + v.x);
  EXPECT_FLOAT_EQ(a.y, font_->getAscent() + font_->getDescent());
  //twice the size, twice as wide (give or take the hinting)
  TextStyle big;
  big.size = 32;
  EXPECT_NEAR(font_->measure("Hello", big).x, 2*font_->measure("Hello").x, 2);
  //lines
  Vec2f lines = font_->measure("A\nAV");
  EXPECT_FLOAT_EQ(lines.x, av.x);
  EXPECT_FLOAT_EQ(lines.y, font_->getLineHeight() + a.y);
  EXPECT_FLOAT_EQ(font_->measure("").x, 0);
}

TEST_F(Text, WrapsAtSpaces){
  float oneTwo = font_->measure("one two").x;
  TextStyle style;
  style.wrapWidth = oneTwo + 1;
  Vec2f wrapped = font_->measure("one two three", style);
  EXPECT_FLOAT_EQ(wrapped.x, oneTwo);
  EXPECT_FLOAT_EQ(wrapped.y, font_->getLineHeight() + font_->measure("x").y);
  //a word longer than the width stays whole
  style.wrapWidth = 5;
  EXPECT_FLOAT_EQ(font_->measure("three", style).x, font_->measure("three").x);
  EXPECT_FLOAT_EQ(font_->measure("one two three", style).y, 2*font_->getLineHeight() + font_->measure("x").y);
}

TEST_F(Text, DrawsOneSpritePerGlyphAndLayer){
  begin();
  //the space has no pixels
  font_->draw(*batch_, 0, 0, "Hi there");
  batch_->flush();
  EXPECT_EQ(null_->getInstanceCount(), 7);
  EXPECT_EQ(null_->getDrawCount(), 1);
  //an outline and a shadow are layers of their own, of the same texture
  TextStyle style;
  style.outline = 2;
  font_->draw(*batch_, 0, 0, "Hi there", style);
  batch_->flush();
  EXPECT_EQ(null_->getInstanceCount(), 7+14);
  style.shadowX = style.shadowY = 2;
  font_->draw(*batch_, 0, 0, "Hi there", style);
  batch_->flush();
  EXPECT_EQ(null_->getInstanceCount(), 7+14+21);
  EXPECT_EQ(null_->getDrawCount(), 3);
  //the batch's settings stay as they were
  EXPECT_EQ(batch_->getSampler(), nullptr);
  EXPECT_LT(batch_->getDistanceField(), 0);
  end();
}

TEST_F(Text, KeepsItsGlyphsInTheAtlas){
  begin();
  font_->draw(*batch_, 0, 0, "aaa");
  EXPECT_EQ(font_->getStats().glyphs, 1);
  EXPECT_EQ(font_->getStats().pages, 1);
  font_->draw(*batch_, 0, 0, "ab");
  EXPECT_EQ(font_->getStats().glyphs, 2);
  //another size and an outline are glyphs of their own
  TextStyle style;
  style.size = 30;
  font_->draw(*batch_, 0, 0, "ab", style);
  EXPECT_EQ(font_->getStats().glyphs, 4);
  style.outline = 1;
  font_->draw(*batch_, 0, 0, "a", style);
  EXPECT_EQ(font_->getStats().glyphs, 5);
  end();
}

TEST_F(Text, DistanceFieldLayersAreDrawsOfTheirOwn){
  FontDesc desc;
  desc.distanceField = true;
  desc.size = 64;
  std::unique_ptr<CGE::Font> font = CGE::Font::load(*device_, engineFont("DejaVuSans.ttf"), desc);
  ASSERT_NE(font, nullptr);
  begin();
  //one set of glyphs for all sizes
  TextStyle style;
  font->draw(*batch_, 0, 0, "ab", style);
  style.size = 200;
  font->draw(*batch_, 0, 0, "ab", style);
  EXPECT_EQ(font->getStats().glyphs, 2);
  batch_->flush();
  EXPECT_EQ(null_->getDrawCount(), 1);
  //the outline is the same glyphs with another edge
  style.outline = 3;
  font->draw(*batch_, 0, 0, "ab", style);
  batch_->flush();
  EXPECT_EQ(font->getStats().glyphs, 2);
  EXPECT_EQ(null_->getDrawCount(), 3);
  EXPECT_EQ(null_->getInstanceCount(), 8);
  EXPECT_LT(batch_->getDistanceField(), 0);
  end();
  font.reset();
}

TEST_F(Text, SyntheticStylesWidenTheGlyphs){
  FontDesc desc;
  desc.embolden = true;
  std::unique_ptr<CGE::Font> bold = CGE::Font::load(*device_, engineFont("DejaVuSans.ttf"), desc);
  ASSERT_NE(bold, nullptr);
  EXPECT_GT(bold->measure("mmmm").x, font_->measure("mmmm").x);
}

TEST(SystemFonts, FindsTheSansSerifFont){
  std::vector<std::string> families = SystemFonts::getFamilies();
  if (families.empty())
    GTEST_SKIP() << "no system fonts";
  EXPECT_TRUE(std::is_sorted(families.begin(), families.end(), [](const std::string& a, const std::string& b){
    std::string la = a, lb = b;
    std::transform(la.begin(), la.end(), la.begin(), ::tolower);
    std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
    return la < lb;
  }));
  SystemFonts::FontData data;
  ASSERT_TRUE(SystemFonts::find("sans-serif", data));
  EXPECT_FALSE(data.data.empty());
  EXPECT_FALSE(data.family.empty());
  std::unique_ptr<RenderDevice> device = RenderDevice::create("Null");
  ASSERT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  FontDesc desc;
  desc.faceIndex = data.faceIndex;
  std::unique_ptr<CGE::Font> font = CGE::Font::load(*device, data.data, desc);
  ASSERT_NE(font, nullptr);
  EXPECT_EQ(font->getFamily(), data.family);
  EXPECT_TRUE(font->hasGlyph(U'A'));
}

TEST(SystemFonts, KnowsTheInstalledFamilies){
  std::vector<std::string> families = SystemFonts::getFamilies();
  if (families.empty())
    GTEST_SKIP() << "no system fonts";
  EXPECT_TRUE(SystemFonts::exists(families.front()));
  EXPECT_FALSE(SystemFonts::exists("No Such Family 4711"));
  EXPECT_FALSE(SystemFonts::exists(""));
}

TEST(SystemFonts, FindsBoldAndItalicFaces){
  SystemFonts::FontData regular, bold;
  if (!SystemFonts::find("serif", regular) || !SystemFonts::find("serif", bold, 700, true))
    GTEST_SKIP() << "no serif font";
  std::unique_ptr<RenderDevice> device = RenderDevice::create("Null");
  ASSERT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  FontDesc desc;
  desc.faceIndex = bold.faceIndex;
  std::unique_ptr<CGE::Font> font = CGE::Font::load(*device, bold.data, desc);
  ASSERT_NE(font, nullptr);
  //another face, or at least another file
  EXPECT_TRUE(regular.data != bold.data || regular.faceIndex != bold.faceIndex);
}

#ifdef WIN32
TEST(SystemFonts, FindsTheFaceInACollection){
  //Cambria is the first face of cambria.ttc, Cambria Math the second
  if (!SystemFonts::exists("Cambria Math"))
    GTEST_SKIP() << "no Cambria Math";
  SystemFonts::FontData data;
  ASSERT_TRUE(SystemFonts::find("Cambria Math", data));
  EXPECT_EQ(data.family, "Cambria Math");
  EXPECT_EQ(data.faceIndex, 1);
  std::unique_ptr<RenderDevice> device = RenderDevice::create("Null");
  ASSERT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  FontDesc desc;
  desc.faceIndex = data.faceIndex;
  std::unique_ptr<CGE::Font> font = CGE::Font::load(*device, data.data, desc);
  ASSERT_NE(font, nullptr);
  EXPECT_EQ(font->getFamily(), "Cambria Math");
}
#endif

#if defined(CGE_TEST_WINDOW)

namespace{

struct Pixel{
  uint8_t r, g, b, a;
};

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

//! the box of the pixels of which a channel is over a threshold
struct Ink{
  int count = 0;
  int minX = 1 << 20;
  int minY = 1 << 20;
  int maxX = -1;
  int maxY = -1;
  float sumX = 0;
  void add(int x, int y){
    ++count;
    minX = (std::min)(minX, x);
    minY = (std::min)(minY, y);
    maxX = (std::max)(maxX, x);
    maxY = (std::max)(maxY, y);
    sumX += x;
  }
  float centreX() const {return count ? sumX/count : 0;}
};

class TextGL : public ::testing::Test{
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
    font_ = CGE::Font::load(*device_, engineFont("DejaVuSans.ttf"));
    ASSERT_NE(font_, nullptr);
    CGE::TraceManager::instance()->setTraceOutputter(&collector_);
    collector_.problems.clear();
    collecting_ = true;
  }

  void TearDown() override{
    font_.reset();
    batch_.reset();
    ctx_.reset();
    if (collecting_){
      CGE::TraceManager::instance()->removeTraceOutputter(&collector_);
      for (const std::string& problem : collector_.problems)
        ADD_FAILURE() << "traced: " << problem;
    }
  }

  //! a frame and a black pass into a target, with a projection of units pixels per unit, y down
  void begin(int width, int height, float units = 1){
    RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.depthFormat = Format::Undefined;
    target_ = device_->createRenderTarget(desc);
    ASSERT_NE(target_, nullptr);
    ASSERT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target_.get();
    pass.clearColor = {0, 0, 0, 1};
    ASSERT_TRUE(device_->beginPass(pass));
    ctx_->setProjection(device_->ortho(0, width/units, height/units, 0, -10, 10));
    ctx_->setView(Matrix(Matrix::Identity));
  }

  //! ends the frame and reads the pixels, rows from the top down
  void end(){
    batch_->flush();
    device_->endPass();
    const RenderTargetDesc& desc = target_->getDesc();
    std::vector<Pixel> bottomUp(desc.width*desc.height);
    EXPECT_TRUE(device_->readPixels(target_.get(), 0, {0, 0, desc.width, desc.height}, bottomUp.data()));
    device_->endFrame();
    width_ = desc.width;
    pixels_.resize(bottomUp.size());
    for (int y = 0; y < desc.height; ++y){
      for (int x = 0; x < desc.width; ++x)
        pixels_[y*desc.width+x] = bottomUp[(desc.height-1-y)*desc.width+x];
    }
    target_.reset();
  }

  //! where a channel (0 red, 1 green, 2 blue) is over the threshold
  Ink ink(int channel, int threshold = 128) const{
    Ink result;
    for (size_t i = 0; i < pixels_.size(); ++i){
      const uint8_t* p = &pixels_[i].r;
      if (p[channel] > threshold)
        result.add((int)i % width_, (int)i / width_);
    }
    return result;
  }

  std::unique_ptr<RenderDevice> device_;
  TestEnv::TestWindow window_;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<SpriteBatch> batch_;
  std::unique_ptr<CGE::Font> font_;
  std::unique_ptr<RenderTarget> target_;
  std::vector<Pixel> pixels_;
  int width_ = 0;
  ProblemCollector collector_;
  bool collecting_ = false;
};

}

TEST_F(TextGL, DrawsTheGlyphsInTheirColourBelowY){
  begin(64, 32);
  TextStyle style;
  style.color = Color(1, 0, 0, 1);
  style.size = 20;
  font_->draw(*batch_, 10, 4, "IH", style);
  end();
  Ink red = ink(0);
  EXPECT_GT(red.count, 40);
  EXPECT_EQ(ink(1).count + ink(2).count, 0);
  //the capitals stand on the baseline, which is the ascent below the top
  float baseline = 4 + font_->getAscent(20);
  EXPECT_GE(red.minX, 10);
  EXPECT_LE(red.maxX, 10 + font_->measure("IH", style).x);
  EXPECT_NEAR(red.maxY + 1, baseline, 1.0f);
  //cap height of DejaVu Sans: 0.73 em
  EXPECT_NEAR(red.maxY + 1 - red.minY, 0.73f*20, 1.5f);
}

TEST_F(TextGL, AlignsTheLinesAtX){
  TextStyle style;
  style.color = Color(1, 0, 0, 1);
  style.size = 20;
  style.align = TextAlign::Center;
  begin(64, 64);
  font_->draw(*batch_, 32, 4, "HH\nH", style);
  end();
  //the ink of both lines is centred
  EXPECT_NEAR(ink(0).centreX(), 32, 1.0f);
  style.align = TextAlign::Right;
  begin(64, 32);
  font_->draw(*batch_, 60, 4, "HH", style);
  end();
  EXPECT_LE(ink(0).maxX, 60);
  EXPECT_GE(ink(0).maxX, 56);
}

TEST_F(TextGL, DrawsTheOutlineAroundAndTheShadowBehind){
  begin(64, 40);
  TextStyle style;
  style.color = Color(1, 0, 0, 1);
  style.size = 24;
  style.outline = 2;
  style.outlineColor = Color(0, 1, 0, 1);
  style.shadowX = 4;
  style.shadowY = 4;
  style.shadowColor = Color(0, 0, 1, 1);
  font_->draw(*batch_, 10, 4, "I", style);
  end();
  Ink red = ink(0);
  Ink green = ink(1);
  Ink blue = ink(2);
  ASSERT_GT(red.count, 10);
  //the outline is about 2 pixels around the glyph, which covers it
  EXPECT_NEAR(green.minX, red.minX - 2, 1);
  EXPECT_NEAR(green.maxX, red.maxX + 2, 1);
  EXPECT_NEAR(green.minY, red.minY - 2, 1);
  //the shadow is the outlined glyph, 4 pixels further, behind it
  EXPECT_NEAR(blue.maxX, green.maxX + 4, 1);
  EXPECT_NEAR(blue.maxY, green.maxY + 4, 1);
  EXPECT_GT(green.count, 10);
}

TEST_F(TextGL, RendersTheGlyphsAtTheScreensScale){
  //two pixels per unit: the same text covers twice the pixels, rendered anew
  TextStyle style;
  style.color = Color(1, 0, 0, 1);
  style.size = 12;
  begin(64, 32);
  font_->draw(*batch_, 2, 2, "H", style);
  end();
  Ink once = ink(0);
  int glyphs = font_->getStats().glyphs;
  begin(64, 32, 2);
  font_->draw(*batch_, 2, 2, "H", style);
  end();
  Ink twice = ink(0);
  EXPECT_EQ(font_->getStats().glyphs, glyphs+1);
  EXPECT_NEAR(twice.maxY - twice.minY + 1, 2*(once.maxY - once.minY + 1), 2);
  //normal glyphs sit on whole pixels: the stems are solid, without half-covered columns
  int solid = 0;
  int row = twice.minY + (twice.maxY-twice.minY)/4;
  for (int x = twice.minX; x <= twice.maxX; ++x){
    const Pixel& p = pixels_[row*width_ + x];
    solid += p.r > 250;
  }
  EXPECT_GE(solid, 2);
}

TEST_F(TextGL, DistanceFieldsAreSharpAtLargeSizes){
  FontDesc desc;
  desc.distanceField = true;
  desc.size = 96;
  std::unique_ptr<CGE::Font> font = CGE::Font::load(*device_, engineFont("DejaVuSans.ttf"), desc);
  ASSERT_NE(font, nullptr);
  begin(96, 96);
  TextStyle style;
  style.color = Color(1, 0, 0, 1);
  font->draw(*batch_, 20, 0, "I", style);
  end();
  Ink red = ink(0);
  ASSERT_GT(red.count, 100);
  //a row through the stem: full inside, black outside, a sharp edge
  int row = (red.minY + red.maxY)/2;
  int partial = 0;
  for (int x = 0; x < 96; ++x){
    const Pixel& p = pixels_[row*width_ + x];
    partial += p.r > 20 && p.r < 235;
  }
  EXPECT_LE(partial, 4);
  EXPECT_GT(pixels_[row*width_ + (red.minX+red.maxX)/2].r, 250);
  EXPECT_NEAR(red.maxY + 1 - red.minY, 0.73f*96, 3);
  //an outline is the shape grown by its width
  begin(96, 96);
  style.outline = 4;
  style.outlineColor = Color(0, 1, 0, 1);
  font->draw(*batch_, 20, 0, "I", style);
  end();
  Ink green = ink(1);
  EXPECT_NEAR(green.minX, red.minX - 4, 1.5f);
  EXPECT_NEAR(green.maxX, red.maxX + 4, 1.5f);
  font.reset();
}

#endif
