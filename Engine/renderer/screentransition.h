#ifndef CGE_SCREENTRANSITION_H
#define CGE_SCREENTRANSITION_H

#include <cge_export.h>

#include <memory>
#include <vector>

#include "../math/vector.h"
#include "spritebatch.h"

namespace CGE{
namespace Gfx{

class RenderDevice;
class RenderTarget;
class Texture;

//! the kinds of screen transitions (the Adventure's screen changes)
enum class TransitionKind{
  //! at once
  Cut,
  //! to black and back
  FadeOut,
  //! a rectangle of the screen's shape shrinks to the centre, then grows again
  Rectangle,
  //! the same with an ellipse, with a soft edge
  Circle,
  //! bands across the screen close upwards, the lower ones later, then open
  Shutters,
  //! black sweeps counterclockwise round from 12 o'clock, then a hand uncovers the screen clockwise
  Clock,
  //! the old scene fades out over the new one
  Blend,
};
//! the number of TransitionKind values
static const int TRANSITION_KINDS = 7;
//! the kind's name ("cut", "fade out", ...)
CGE_API const char* getTransitionName(TransitionKind kind);

//! a wedge of the clock face, in degrees clockwise from 12 o'clock (from < to)
struct TransitionWedge{
  float from = 0;
  float to = 0;
};

//! a parallelogram: the corners origin, origin+a, origin+b and origin+a+b
struct CGE_API Parallelogram{
  Vec2f origin;
  Vec2f a;
  Vec2f b;
  bool contains(const Vec2f& p) const;
};

//! A screen transition's timing and the shapes of its masks
/*! Most kinds close over the old scene in the first half of their time and
    open over the new one in the second; the scenes change in the middle,
    while the screen is black. A blend fades a picture of the old scene out
    over the new one, so the scenes change at its start, and so does a cut.
    The user advances it, changes the scenes when isChangeDue() says so and
    then calls changed(); the opening half takes its full time from then on.
    The masks are in screen units with y down: a rectangle or an ellipse of
    the screen's shape around its centre, bands across the screen, or the
    wedges of a clock face. ScreenTransitionRenderer draws them. */
class CGE_API ScreenTransition{
public:
  enum class Phase{
    //! over the old scene
    Closing,
    //! over the new one
    Opening,
    Done,
  };
  //! the number of bands of the shutters
  static const int SHUTTERS = 10;

  //! no transition (done)
  ScreenTransition() = default;
  ScreenTransition(TransitionKind kind, double seconds);

  TransitionKind getKind() const {return kind_;}
  //! seconds (0 for a cut)
  double getDuration() const {return duration_;}
  //! the seconds since it began
  double getElapsed() const {return elapsed_;}

  void advance(double seconds);
  Phase getPhase() const;
  bool isDone() const {return getPhase() == Phase::Done;}
  //! whether the scenes have to change now: from the end of the closing
  //! half until changed() (at once for a cut and a blend)
  bool isChangeDue() const {return !changed_ && elapsed_ >= getClosingTime();}
  //! the scenes changed: the opening half begins
  void changed();
  //! a blend needs a picture of the old scene before the scenes change
  bool needsPicture() const {return kind_ == TransitionKind::Blend;}

  //! how far the screen is open: from 1 to 0 while closing, from 0 to 1
  //! while opening, 1 when done; for a blend the new scene's share
  float getOpenness() const;

  //! the half size of the rectangle or ellipse that shows the scene; it
  //! uncovers the corners of a screen of width x height just when open
  Vec2f getMaskHalfSize(float width, float height) const;
  //! how much of the band (0 at the top, of SHUTTERS) shows the scene, from
  //! 0 to 1; the shown part is at the band's bottom
  float getShutterOpen(int band) const;
  //! the black parts of the clock face, in wedges of at most 90 degrees;
  //! the scene shows from 0 to openness*360 degrees
  std::vector<TransitionWedge> getClockWedges() const;

  //! the direction of an angle of the clock face (degrees clockwise from 12
  //! o'clock), with y down
  static Vec2f getClockDirection(float degrees);
  //! a parallelogram that covers a wedge of at most 90 degrees around centre
  //! up to reach from it, and nothing outside the wedge
  static Parallelogram getWedgeShape(const Vec2f& centre, const TransitionWedge& wedge, float reach);

private:
  double getClosingTime() const;

  TransitionKind kind_ = TransitionKind::Cut;
  double duration_ = 0;
  double elapsed_ = 0;
  bool changed_ = true;
  //! when the scenes changed (the opening half's start)
  double changedAt_ = 0;
};

//! an area of the screen in the batch's coordinates (y down)
struct ScreenArea{
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;
};

//! a picture of the old scene: a texture and the part of it to show
struct TransitionPicture{
  Texture* texture = nullptr;
  TexRect rect;
};

//! Draws screen transitions with a sprite batch
/*! The masks are drawn over what was drawn before, in one colour: black
    rectangles, the ellipse's hole (a small texture made at the first use,
    scaled up, so its edge is soft) and the clock's wedges as
    parallelograms. A picture of the old scene, if given, is drawn under the
    mask while closing; a blend fades it out while opening. The picture's
    alpha must be 1 (makeOpaque).
    The renderer can keep a render target for the picture
    (getPictureTarget). Delete it before the device. */
class CGE_API ScreenTransitionRenderer{
public:
  explicit ScreenTransitionRenderer(RenderDevice& device);
  ~ScreenTransitionRenderer();
  ScreenTransitionRenderer(const ScreenTransitionRenderer&) = delete;
  ScreenTransitionRenderer& operator=(const ScreenTransitionRenderer&) = delete;

  //! a render target of width x height without depth for a picture of the
  //! old scene, made at the first call and again when the size changes;
  //! nullptr if it can't be made
  RenderTarget* getPictureTarget(int width, int height);
  //! the picture target's colour, the right way up with a projection whose
  //! y points down (OpenGL's render targets have their bottom row first)
  TransitionPicture getPicture() const;
  //! sets the alpha of what was drawn into area of the current target to 1
  //! and keeps its colour (the blend modes also blend the alpha, so it is
  //! below 1 where translucent sprites overlap); for a picture before it is
  //! used. Draws into the batch, with its blend mode restored.
  static void makeOpaque(SpriteBatch& batch, const ScreenArea& area);

  //! draws the transition over area into the batch, with the picture of the
  //! old scene if there is one; the batch's blend mode is restored
  void draw(const ScreenTransition& transition, SpriteBatch& batch, const ScreenArea& area,
    const TransitionPicture* oldScene = nullptr);

private:
  void black(SpriteBatch& batch, float x, float y, float width, float height) const;
  void makeHole();

  RenderDevice& device_;
  std::unique_ptr<RenderTarget> picture_;
  std::unique_ptr<Texture> hole_;
};

}
}

#endif
