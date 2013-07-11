#include "line.h"
#include "tristrip.h"

using namespace CGE;

void Line::convert(TriStrip& strip){
  Vec2f dir = mEnd-mStart;
  Vec2f ortho = dir.cross().normalize()*mWidth;
  strip.addVertex(mStart-ortho);
  strip.addVertex(mEnd-ortho);
  strip.addVertex(mStart+ortho);
  strip.addVertex(mEnd+ortho);
}
