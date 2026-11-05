#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

#include <math/matrix.h>
#include <renderer/drawcontext.h>
#include <renderer/renderdevice.h>
#include <renderer/screentransition.h>
#include <renderer/spritebatch.h>

#include "testwindow.h"

using namespace CGE;
using namespace CGE::Gfx;
using Phase = ScreenTransition::Phase;

// ScreenTransition's timing and masks, and ScreenTransitionRenderer, whose
// masks are drawn into render targets on a hidden window and read back.

namespace{

//! the angle of a point around a centre, degrees clockwise from 12 o'clock (y down), from 0 to 360
float clockAngle(const Vec2f& centre, const Vec2f& p){
  float a = std::atan2(p.x - centre.x, -(p.y - centre.y))*180/3.14159265f;
  return a < 0 ? a + 360 : a;
}

}

TEST(ScreenTransition, DefaultIsDone){
  ScreenTransition t;
  EXPECT_TRUE(t.isDone());
  EXPECT_FALSE(t.isChangeDue());
  EXPECT_FLOAT_EQ(t.getOpenness(), 1);
}

TEST(ScreenTransition, CutChangesAtOnce){
  ScreenTransition t(TransitionKind::Cut, 2);
  EXPECT_EQ(t.getDuration(), 0);
  EXPECT_EQ(t.getPhase(), Phase::Closing);
  EXPECT_TRUE(t.isChangeDue());
  t.changed();
  EXPECT_FALSE(t.isChangeDue());
  EXPECT_TRUE(t.isDone());
}

TEST(ScreenTransition, ClosesThenOpens){
  ScreenTransition t(TransitionKind::FadeOut, 1);
  EXPECT_EQ(t.getPhase(), Phase::Closing);
  EXPECT_FLOAT_EQ(t.getOpenness(), 1);
  EXPECT_FALSE(t.isChangeDue());
  t.advance(0.25);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0.5f);
  EXPECT_FALSE(t.isChangeDue());
  t.advance(0.25);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0);
  EXPECT_TRUE(t.isChangeDue());
  //it waits for the change, black
  t.advance(0.1);
  EXPECT_EQ(t.getPhase(), Phase::Closing);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0);
  EXPECT_TRUE(t.isChangeDue());
  t.changed();
  EXPECT_FALSE(t.isChangeDue());
  //the opening half takes its full time from the change on
  EXPECT_EQ(t.getPhase(), Phase::Opening);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0);
  t.advance(0.25);
  EXPECT_NEAR(t.getOpenness(), 0.5f, 1e-5);
  t.advance(0.2);
  EXPECT_EQ(t.getPhase(), Phase::Opening);
  t.advance(0.06);
  EXPECT_TRUE(t.isDone());
  EXPECT_FLOAT_EQ(t.getOpenness(), 1);
}

TEST(ScreenTransition, ChangedOnTimeOpensFromTheMiddle){
  ScreenTransition t(TransitionKind::Circle, 2);
  t.advance(1.0);
  ASSERT_TRUE(t.isChangeDue());
  t.changed();
  t.advance(0.5);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0.5f);
  t.advance(0.5);
  EXPECT_TRUE(t.isDone());
}

TEST(ScreenTransition, BlendChangesAtOnceAndFadesIn){
  ScreenTransition t(TransitionKind::Blend, 0.5);
  EXPECT_TRUE(t.needsPicture());
  EXPECT_TRUE(t.isChangeDue());
  //the new scene's share
  EXPECT_FLOAT_EQ(t.getOpenness(), 0);
  t.changed();
  EXPECT_EQ(t.getPhase(), Phase::Opening);
  t.advance(0.25);
  EXPECT_FLOAT_EQ(t.getOpenness(), 0.5f);
  t.advance(0.25);
  EXPECT_TRUE(t.isDone());
  EXPECT_FALSE(ScreenTransition(TransitionKind::Clock, 1).needsPicture());
}

TEST(ScreenTransition, MaskUncoversTheCornersWhenOpen){
  const float w = 1920, h = 1080;
  for (TransitionKind kind : {TransitionKind::Rectangle, TransitionKind::Circle}){
    ScreenTransition t(kind, 1);
    Vec2f half = t.getMaskHalfSize(w, h);
    if (kind == TransitionKind::Rectangle){
      EXPECT_FLOAT_EQ(half.x, w/2);
      EXPECT_FLOAT_EQ(half.y, h/2);
    }
    else{
      //a corner on the ellipse
      float e = (w/2)*(w/2)/(half.x*half.x) + (h/2)*(h/2)/(half.y*half.y);
      EXPECT_NEAR(e, 1, 1e-4);
    }
    //the screen's shape, shrinking to nothing
    t.advance(0.25);
    Vec2f quarter = t.getMaskHalfSize(w, h);
    EXPECT_NEAR(quarter.x, half.x/2, 1e-3);
    EXPECT_NEAR(quarter.x/quarter.y, w/h, 1e-4);
    t.advance(0.25);
    EXPECT_FLOAT_EQ(t.getMaskHalfSize(w, h).x, 0);
  }
}

TEST(ScreenTransition, ShuttersLowerBandsCloseLater){
  ScreenTransition t(TransitionKind::Shutters, 1);
  for (int band = 0; band < ScreenTransition::SHUTTERS; ++band)
    EXPECT_FLOAT_EQ(t.getShutterOpen(band), 1);
  t.advance(0.25);
  //openness 0.5: the top band half open, the lowest one still open
  EXPECT_FLOAT_EQ(t.getShutterOpen(0), 0.5f);
  EXPECT_FLOAT_EQ(t.getShutterOpen(ScreenTransition::SHUTTERS-1), 1);
  for (int band = 1; band < ScreenTransition::SHUTTERS; ++band)
    EXPECT_GE(t.getShutterOpen(band), t.getShutterOpen(band-1));
  t.advance(0.25);
  for (int band = 0; band < ScreenTransition::SHUTTERS; ++band)
    EXPECT_FLOAT_EQ(t.getShutterOpen(band), 0);
}

TEST(ScreenTransition, ClockWedgesCoverTheRest){
  ScreenTransition t(TransitionKind::Clock, 1);
  EXPECT_TRUE(t.getClockWedges().empty());
  //openness from 1 down to 0 in steps
  for (int i = 1; i <= 50; ++i){
    t.advance(0.01);
    float start = t.getOpenness()*360;
    std::vector<TransitionWedge> wedges = t.getClockWedges();
    ASSERT_FALSE(wedges.empty());
    EXPECT_FLOAT_EQ(wedges.front().from, start);
    EXPECT_FLOAT_EQ(wedges.back().to, 360);
    for (size_t k = 0; k < wedges.size(); ++k){
      EXPECT_LE(wedges[k].to - wedges[k].from, 90.0f + 1e-3f);
      EXPECT_GT(wedges[k].to, wedges[k].from);
      if (k > 0)
        EXPECT_FLOAT_EQ(wedges[k].from, wedges[k-1].to);
    }
  }
  EXPECT_FLOAT_EQ(t.getOpenness(), 0);
  EXPECT_EQ(t.getClockWedges().size(), 4u);
}

TEST(ScreenTransition, ClockDirections){
  EXPECT_NEAR(ScreenTransition::getClockDirection(0).x, 0, 1e-6);
  EXPECT_NEAR(ScreenTransition::getClockDirection(0).y, -1, 1e-6);
  EXPECT_NEAR(ScreenTransition::getClockDirection(90).x, 1, 1e-6);
  EXPECT_NEAR(ScreenTransition::getClockDirection(180).y, 1, 1e-6);
  EXPECT_NEAR(ScreenTransition::getClockDirection(270).x, -1, 1e-6);
}

TEST(ScreenTransition, WedgeShapesCoverExactlyTheBlackPart){
  //points on a grid over a 1920x1080 screen: black if and only if their
  //angle is in the black part (away from its edges)
  const Vec2f centre(960, 540);
  const float reach = std::hypot(960.0f, 540.0f) + 2;
  for (double opened : {0.0, 0.1, 0.3, 0.5, 0.74, 0.9, 0.99}){
    ScreenTransition t(TransitionKind::Clock, 1);
    t.advance(0.5*(1 - opened));
    float start = t.getOpenness()*360;
    std::vector<Parallelogram> shapes;
    for (const TransitionWedge& w : t.getClockWedges())
      shapes.push_back(ScreenTransition::getWedgeShape(centre, w, reach));
    int black = 0, wrong = 0;
    for (float y = 4; y < 1080; y += 23){
      for (float x = 4; x < 1920; x += 23){
        Vec2f p(x, y);
        float a = clockAngle(centre, p);
        //a degree of margin at the edges, and none at the centre
        if (std::hypot(x - centre.x, y - centre.y) < 30 || std::abs(a - start) < 1 || a < 1 || a > 359)
          continue;
        bool covered = false;
        for (const Parallelogram& s : shapes)
          covered = covered || s.contains(p);
        bool expected = a > start;
        black += expected;
        wrong += covered != expected;
      }
    }
    EXPECT_EQ(wrong, 0) << "opened " << opened;
    if (opened < 0.9)
      EXPECT_GT(black, 100);
  }
}

TEST(ScreenTransition, ParallelogramContains){
  Parallelogram p;
  p.origin = Vec2f(10, 10);
  p.a = Vec2f(100, 0);
  p.b = Vec2f(20, 50);
  EXPECT_TRUE(p.contains(Vec2f(11, 11)));
  EXPECT_TRUE(p.contains(Vec2f(125, 55)));
  EXPECT_FALSE(p.contains(Vec2f(9, 11)));
  EXPECT_FALSE(p.contains(Vec2f(20, 61)));
  EXPECT_FALSE(p.contains(Vec2f(131, 59)));
  //a flat one contains nothing
  Parallelogram flat;
  flat.a = Vec2f(10, 0);
  flat.b = Vec2f(20, 0);
  EXPECT_FALSE(flat.contains(Vec2f(5, 0)));
}

TEST(ScreenTransition, Names){
  for (int i = 0; i < TRANSITION_KINDS; ++i)
    EXPECT_STRNE(getTransitionName((TransitionKind)i), "?");
}

#if defined(CGE_TEST_WINDOW)

namespace{

using TestEnv::TestWindow;

struct Pixel{
  uint8_t r, g, b, a;
};

::testing::AssertionResult isNear(const Pixel& p, int r, int g, int b, int tolerance = 3){
  if (abs(p.r-r) <= tolerance && abs(p.g-g) <= tolerance && abs(p.b-b) <= tolerance)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "pixel (" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b
    << ") instead of (" << r << ", " << g << ", " << b << ")";
}

const int W = 40;
const int H = 30;

//! a white scene of 40x30 with a transition drawn over it
class TransitionPixels : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("OpenGL");
    ASSERT_NE(device_, nullptr);
    DeviceDesc desc;
    if (!window_.create(*device_, desc))
      GTEST_SKIP() << "no window";
    if (!device_->init(window_.native(), desc))
      GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
    ctx_ = std::make_unique<DrawContext>(*device_);
    batch_ = std::make_unique<SpriteBatch>(*ctx_);
    renderer_ = std::make_unique<ScreenTransitionRenderer>(*device_);
    RenderTargetDesc rt;
    rt.width = W;
    rt.height = H;
    rt.depthFormat = Format::Undefined;
    target_ = device_->createRenderTarget(rt);
    area_.width = W;
    area_.height = H;
  }

  void TearDown() override{
    target_.reset();
    renderer_.reset();
    batch_.reset();
    ctx_.reset();
  }

  //! a frame and a pass into target cleared to the colour, with a projection in pixels, y down
  void begin(RenderTarget* target, ClearColor clear = {0, 0, 0, 1}){
    ASSERT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target;
    pass.clearColor = clear;
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

  //! the white scene with the transition over it, read back
  std::vector<Pixel> drawOverWhite(const ScreenTransition& t, const TransitionPicture* picture = nullptr){
    begin(target_.get());
    batch_->draw(nullptr, 0, 0, (float)W, (float)H, TexRect(), Color(1, 1, 1, 1));
    renderer_->draw(t, *batch_, area_, picture);
    end();
    return read(target_.get());
  }

  //! the pixels, rows from the top down
  std::vector<Pixel> read(RenderTarget* target){
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

  static const Pixel& at(const std::vector<Pixel>& pixels, int x, int y){
    return pixels[y*W+x];
  }

  //declared first, so the device goes before the window
  TestWindow window_;
  std::unique_ptr<RenderDevice> device_;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<SpriteBatch> batch_;
  std::unique_ptr<ScreenTransitionRenderer> renderer_;
  std::unique_ptr<RenderTarget> target_;
  ScreenArea area_;
};

//! a 2x2 texture of one colour
std::unique_ptr<Texture> plain(RenderDevice& device, uint8_t r, uint8_t g, uint8_t b){
  std::vector<uint8_t> data;
  for (int i = 0; i < 4; ++i){
    data.push_back(r);
    data.push_back(g);
    data.push_back(b);
    data.push_back(255);
  }
  TextureDesc desc;
  desc.width = desc.height = 2;
  return device.createTexture(desc, data.data());
}

}

TEST_F(TransitionPixels, FadeOutDarkens){
  ScreenTransition t(TransitionKind::FadeOut, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 0, 0), 128, 128, 128));
  EXPECT_TRUE(isNear(at(p, 20, 15), 128, 128, 128));
}

TEST_F(TransitionPixels, RectangleShowsTheMiddle){
  //openness 0.5: half of the half screen, x 10 to 30 and y 7.5 to 22.5
  ScreenTransition t(TransitionKind::Rectangle, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 11, 8), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 29, 22), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 8, 15), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 20, 5), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 31, 24), 0, 0, 0));
}

TEST_F(TransitionPixels, CircleShowsAnEllipse){
  //openness 0.5: semi-axes of 14.1 and 10.6 around (20, 15)
  ScreenTransition t(TransitionKind::Circle, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 27, 15), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 20, 20), 255, 255, 255));
  //the corners, and the corners of the ellipse's bounds
  EXPECT_TRUE(isNear(at(p, 0, 0), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 39, 29), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 8, 6), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 32, 24), 0, 0, 0));
}

TEST_F(TransitionPixels, ClockShowsTheRightHalf){
  //openness 0.5 while closing: the scene from 12 to 6 o'clock
  ScreenTransition t(TransitionKind::Clock, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 30, 5), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 30, 25), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 39, 0), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 10, 5), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 10, 25), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 0, 29), 0, 0, 0));
}

TEST_F(TransitionPixels, ShuttersCloseFromTheTop){
  //openness 0.5: the top band (3 rows) black in its upper half, the lowest open
  ScreenTransition t(TransitionKind::Shutters, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 20, 0), 0, 0, 0));
  EXPECT_TRUE(isNear(at(p, 20, 2), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 20, 27), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 20, 29), 255, 255, 255));
}

TEST_F(TransitionPixels, ThePictureIsUnderTheMaskWhileClosing){
  std::unique_ptr<Texture> green = plain(*device_, 0, 255, 0);
  TransitionPicture picture;
  picture.texture = green.get();
  ScreenTransition t(TransitionKind::Circle, 1);
  t.advance(0.25);
  std::vector<Pixel> p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 15), 0, 255, 0));
  EXPECT_TRUE(isNear(at(p, 0, 0), 0, 0, 0));
  //opening, the new scene shows in the hole
  t.advance(0.25);
  t.changed();
  t.advance(0.25);
  p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 0, 0), 0, 0, 0));
  green.reset();
}

TEST_F(TransitionPixels, BlendFadesThePictureOut){
  std::unique_ptr<Texture> red = plain(*device_, 255, 0, 0);
  TransitionPicture picture;
  picture.texture = red.get();
  ScreenTransition t(TransitionKind::Blend, 1);
  //before the change only the picture shows
  std::vector<Pixel> p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 0, 0));
  t.changed();
  t.advance(0.5);
  p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 128, 128));
  t.advance(0.5);
  p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 255, 255));
  red.reset();
}

TEST_F(TransitionPixels, ThePictureTargetIsUpright){
  //red in the top half of the picture, blue below, drawn with y down
  RenderTarget* pictureTarget = renderer_->getPictureTarget(4, 4);
  ASSERT_NE(pictureTarget, nullptr);
  EXPECT_EQ(renderer_->getPictureTarget(4, 4), pictureTarget);
  begin(pictureTarget);
  batch_->draw(nullptr, 0, 0, 4, 2, TexRect(), Color(1, 0, 0, 1));
  batch_->draw(nullptr, 0, 2, 4, 2, TexRect(), Color(0, 0, 1, 1));
  end();
  TransitionPicture picture = renderer_->getPicture();
  ASSERT_NE(picture.texture, nullptr);
  ScreenTransition t(TransitionKind::Blend, 1);
  std::vector<Pixel> p = drawOverWhite(t, &picture);
  EXPECT_TRUE(isNear(at(p, 20, 3), 255, 0, 0));
  EXPECT_TRUE(isNear(at(p, 20, 26), 0, 0, 255));
  //another size makes a new target
  RenderTarget* other = renderer_->getPictureTarget(8, 4);
  ASSERT_NE(other, nullptr);
  EXPECT_EQ(other->getDesc().width, 8);
}

TEST_F(TransitionPixels, MakeOpaqueSetsTheAlpha){
  //half transparent red on a transparent target: the alpha blends too
  begin(target_.get(), {0, 0, 0, 0});
  batch_->draw(nullptr, 0, 0, (float)W, (float)H, TexRect(), Color(1, 0, 0, 0.5f));
  batch_->flush();
  end();
  std::vector<Pixel> p = read(target_.get());
  EXPECT_LT(at(p, 20, 15).a, 200);
  begin(target_.get(), {0, 0, 0, 0});
  batch_->draw(nullptr, 0, 0, (float)W, (float)H, TexRect(), Color(1, 0, 0, 0.5f));
  batch_->setBlend(BlendMode::Alpha);
  ScreenTransitionRenderer::makeOpaque(*batch_, area_);
  EXPECT_EQ(batch_->getBlend(), BlendMode::Alpha);
  end();
  p = read(target_.get());
  EXPECT_TRUE(isNear(at(p, 20, 15), 128, 0, 0));
  EXPECT_EQ(at(p, 20, 15).a, 255);
}

TEST_F(TransitionPixels, DoneDrawsNothing){
  ScreenTransition t;
  std::vector<Pixel> p = drawOverWhite(t);
  EXPECT_TRUE(isNear(at(p, 0, 0), 255, 255, 255));
  EXPECT_TRUE(isNear(at(p, 20, 15), 255, 255, 255));
}

#endif
