#include "screentransition.h"

#include <algorithm>
#include <cmath>

#include "renderdevice.h"
#include "../math/matrix.h"

using namespace CGE;
using namespace CGE::Gfx;

namespace{

const float PI = 3.14159265358979f;

//! the ellipse's hole: black, transparent inside a circle, with an edge of a few texels
const int HOLE_SIZE = 256;
const float HOLE_EDGE = 3;

const Color BLACK(0, 0, 0, 1);

float clamp01(double v){
  return (float)(std::min)(1.0, (std::max)(0.0, v));
}

}

const char* CGE::Gfx::getTransitionName(TransitionKind kind){
  switch (kind){
    case TransitionKind::Cut: return "cut";
    case TransitionKind::FadeOut: return "fade out";
    case TransitionKind::Rectangle: return "rectangle";
    case TransitionKind::Circle: return "circle";
    case TransitionKind::Shutters: return "shutters";
    case TransitionKind::Clock: return "clock";
    case TransitionKind::Blend: return "blend";
  }
  return "?";
}

bool Parallelogram::contains(const Vec2f& p) const{
  //p - origin = s*a + t*b with s and t from 0 to 1
  float det = a.x*b.y - a.y*b.x;
  if (std::abs(det) < 1e-6f)
    return false;
  float dx = p.x - origin.x;
  float dy = p.y - origin.y;
  float s = (dx*b.y - dy*b.x)/det;
  float t = (a.x*dy - a.y*dx)/det;
  return s >= 0 && s <= 1 && t >= 0 && t <= 1;
}

// --- ScreenTransition ---

ScreenTransition::ScreenTransition(TransitionKind kind, double seconds) :
  kind_(kind), duration_(kind == TransitionKind::Cut ? 0 : (std::max)(0.0, seconds)), changed_(false){
}

double ScreenTransition::getClosingTime() const{
  if (kind_ == TransitionKind::Cut || kind_ == TransitionKind::Blend)
    return 0;
  return duration_/2;
}

void ScreenTransition::advance(double seconds){
  elapsed_ += seconds;
}

void ScreenTransition::changed(){
  if (changed_)
    return;
  changed_ = true;
  //the opening half takes its full time, even if the change came late
  changedAt_ = (std::max)(elapsed_, getClosingTime());
}

ScreenTransition::Phase ScreenTransition::getPhase() const{
  if (!changed_)
    return Phase::Closing;
  return elapsed_ >= changedAt_ + (duration_ - getClosingTime()) ? Phase::Done : Phase::Opening;
}

float ScreenTransition::getOpenness() const{
  switch (getPhase()){
    case Phase::Closing:{
      double closing = getClosingTime();
      //a blend's new scene shows nothing yet
      if (closing <= 0)
        return kind_ == TransitionKind::Blend ? 0.0f : 1.0f;
      return clamp01(1 - elapsed_/closing);
    }
    case Phase::Opening:{
      double opening = duration_ - getClosingTime();
      return opening > 0 ? clamp01((elapsed_ - changedAt_)/opening) : 1.0f;
    }
    default:
      return 1;
  }
}

Vec2f ScreenTransition::getMaskHalfSize(float width, float height) const{
  //the ellipse passes through the corners at 1/sqrt(2) of the screen's
  //size, the rectangle at half of it
  float reach = kind_ == TransitionKind::Circle ? 0.70710678f : 0.5f;
  float open = getOpenness();
  return Vec2f(open*reach*width, open*reach*height);
}

float ScreenTransition::getShutterOpen(int band) const{
  //the lowest band opens twice as fast as the top one
  float speed = 1 + (float)band/(SHUTTERS-1);
  return (std::min)(1.0f, getOpenness()*speed);
}

std::vector<TransitionWedge> ScreenTransition::getClockWedges() const{
  std::vector<TransitionWedge> wedges;
  float start = getOpenness()*360;
  if (start >= 360)
    return wedges;
  int count = (std::max)(1, (int)std::ceil((360 - start)/90 - 1e-4f));
  float step = (360 - start)/count;
  for (int i = 0; i < count; ++i){
    TransitionWedge wedge;
    wedge.from = start + i*step;
    wedge.to = i+1 == count ? 360.0f : start + (i+1)*step;
    wedges.push_back(wedge);
  }
  return wedges;
}

Vec2f ScreenTransition::getClockDirection(float degrees){
  float a = degrees*PI/180;
  return Vec2f(std::sin(a), -std::cos(a));
}

Parallelogram ScreenTransition::getWedgeShape(const Vec2f& centre, const TransitionWedge& wedge, float reach){
  //the parallelogram spanned by the wedge's edges lies inside the wedge; for
  //wedges of at most 90 degrees it covers the wedge up to its edges' length
  //(a point at distance r has coordinates of at most r along them)
  Vec2f from = getClockDirection(wedge.from);
  Vec2f to = getClockDirection(wedge.to);
  Parallelogram p;
  p.origin = centre;
  p.a = Vec2f(from.x*reach, from.y*reach);
  p.b = Vec2f(to.x*reach, to.y*reach);
  return p;
}

// --- ScreenTransitionRenderer ---

ScreenTransitionRenderer::ScreenTransitionRenderer(RenderDevice& device) : device_(device){
}

ScreenTransitionRenderer::~ScreenTransitionRenderer(){
}

RenderTarget* ScreenTransitionRenderer::getPictureTarget(int width, int height){
  if (width <= 0 || height <= 0)
    return nullptr;
  if (!picture_ || picture_->getDesc().width != width || picture_->getDesc().height != height){
    picture_.reset();
    RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.depthFormat = Format::Undefined;
    picture_ = device_.createRenderTarget(desc);
  }
  return picture_.get();
}

TransitionPicture ScreenTransitionRenderer::getPicture() const{
  TransitionPicture picture;
  if (!picture_)
    return picture;
  picture.texture = picture_->getColorTexture(0);
  if (!device_.getClipSpace().originTopLeft)
    std::swap(picture.rect.v0, picture.rect.v1);
  return picture;
}

void ScreenTransitionRenderer::makeOpaque(SpriteBatch& batch, const ScreenArea& area){
  //src*alpha + dst: the colour stays, the alpha becomes at least 1
  BlendMode blend = batch.getBlend();
  batch.setBlend(BlendMode::Additive);
  batch.draw(nullptr, area.x, area.y, area.width, area.height, TexRect(), Color(0, 0, 0, 1));
  batch.setBlend(blend);
}

void ScreenTransitionRenderer::black(SpriteBatch& batch, float x, float y, float width, float height) const{
  if (width > 0 && height > 0)
    batch.draw(nullptr, x, y, width, height, TexRect(), BLACK);
}

void ScreenTransitionRenderer::makeHole(){
  if (hole_)
    return;
  std::vector<unsigned char> pixels(HOLE_SIZE*HOLE_SIZE*4, 0);
  const float centre = (HOLE_SIZE-1)/2.0f;
  const float radius = HOLE_SIZE/2.0f - 1 - HOLE_EDGE;
  for (int y = 0; y < HOLE_SIZE; ++y){
    for (int x = 0; x < HOLE_SIZE; ++x){
      float d = std::hypot(x - centre, y - centre);
      float a = (std::min)(1.0f, (std::max)(0.0f, (d - radius)/HOLE_EDGE));
      pixels[(y*HOLE_SIZE+x)*4+3] = (unsigned char)std::lround(a*255);
    }
  }
  TextureDesc desc;
  desc.width = HOLE_SIZE;
  desc.height = HOLE_SIZE;
  hole_ = device_.createTexture(desc, pixels.data());
}

void ScreenTransitionRenderer::draw(const ScreenTransition& t, SpriteBatch& batch, const ScreenArea& area,
  const TransitionPicture* oldScene){
  if (t.isDone())
    return;
  BlendMode blend = batch.getBlend();
  batch.setBlend(BlendMode::Alpha);
  const float w = area.width;
  const float h = area.height;
  const Vec2f centre(area.x + w/2, area.y + h/2);
  const float open = t.getOpenness();
  const bool closing = t.getPhase() == ScreenTransition::Phase::Closing;

  //the old scene under the mask while closing; a blend fades it out
  if (oldScene && oldScene->texture){
    float alpha = 0;
    if (closing)
      alpha = 1;
    else if (t.getKind() == TransitionKind::Blend)
      alpha = 1 - open;
    if (alpha > 0)
      batch.draw(oldScene->texture, area.x, area.y, w, h, oldScene->rect, Color(1, 1, 1, alpha));
  }

  switch (t.getKind()){
    case TransitionKind::Cut:
    case TransitionKind::Blend:
      break;
    case TransitionKind::FadeOut:
      batch.draw(nullptr, area.x, area.y, w, h, TexRect(), Color(0, 0, 0, 1 - open));
      break;
    case TransitionKind::Rectangle:
    case TransitionKind::Circle:{
      Vec2f half = t.getMaskHalfSize(w, h);
      float left = centre.x - half.x;
      float right = centre.x + half.x;
      float top = centre.y - half.y;
      float bottom = centre.y + half.y;
      //black around the shape's bounds
      black(batch, area.x, area.y, w, top - area.y);
      black(batch, area.x, bottom, w, area.y + h - bottom);
      black(batch, area.x, top, left - area.x, bottom - top);
      black(batch, right, top, area.x + w - right, bottom - top);
      if (half.x <= 0 || half.y <= 0){
        black(batch, area.x, area.y, w, h);
      }
      else if (t.getKind() == TransitionKind::Circle){
        makeHole();
        if (hole_)
          batch.draw(hole_.get(), left, top, right - left, bottom - top, TexRect(), Color(1, 1, 1, 1));
      }
      break;
    }
    case TransitionKind::Shutters:{
      float band = h/ScreenTransition::SHUTTERS;
      for (int i = 0; i < ScreenTransition::SHUTTERS; ++i)
        black(batch, area.x, area.y + i*band, w, band*(1 - t.getShutterOpen(i)));
      break;
    }
    case TransitionKind::Clock:{
      float reach = std::hypot(w, h)/2 + 4;
      for (const TransitionWedge& wedge : t.getClockWedges()){
        Parallelogram p = ScreenTransition::getWedgeShape(centre, wedge, reach);
        //the unit quad's x- and y-axis along the parallelogram's sides (column-major)
        float m[16] = {p.a.x, p.a.y, 0, 0, p.b.x, p.b.y, 0, 0, 0, 0, 1, 0, p.origin.x, p.origin.y, 0, 1};
        batch.draw(nullptr, Matrix(m), TexRect(), BLACK);
      }
      break;
    }
  }
  batch.setBlend(blend);
}
