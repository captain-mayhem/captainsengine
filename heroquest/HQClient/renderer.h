//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: renderer.qt.hh                                             |
//  ==================================================================

#ifndef HQRENDERER_HH
#define HQRENDERER_HH

#include "system/engine.h"

class HQRenderer{
public:
  static void init(Graphics::Renderer* rend){rend_ = new HQRenderer(rend);}
  inline static HQRenderer* instance() {return rend_;}
  inline static void release() {SAFE_DELETE(rend_);}
  inline static void initialize() {rend_->initialize_();}
  inline static void paint() {rend_->paint_();}
protected:
  HQRenderer(Graphics::Renderer* rend);
  void initialize_();
  void paint_();
  static HQRenderer* rend_;
};

#endif
