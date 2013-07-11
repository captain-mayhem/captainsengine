#ifndef SC_LINE_H
#define SC_LINE_H

#include <math/vector.h>

class TriStrip;

class Line{
public:
  Line(const CGE::Vec2f& start, const CGE::Vec2f& end, float width) : mStart(start), mEnd(end),
    mWidth(width) {}
  void convert(TriStrip& strip);
protected:
  CGE::Vec2f mStart;
  CGE::Vec2f mEnd;
  float mWidth;
};

#endif
