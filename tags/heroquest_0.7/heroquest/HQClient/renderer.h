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
#include "math/vector.h"

class Inventory;

//! provides an OpenGL drawing area
/*! Besides drawing the scene, it handles also key and mouse events
 */
class HQRenderer{
public:
  static void init(Graphics::Renderer* rend){rend_ = new HQRenderer(rend);}
  inline static HQRenderer* instance() {return rend_;}
  inline static void release() {SAFE_DELETE(rend_);}
  //! the callback for init the scene
  inline static void initialize() {rend_->initialize_();}
  //! the callback for rendering the scene
  inline static void paint() {rend_->paint_();}
  //! the callback for ascii characters
  inline static void ascii(unsigned char key) {rend_->ascii_(key);}
  //! the callback for special characters
  inline static void special(int key) {rend_->special_(key);}
  //! the callback for key up
  inline static void keyUp(int key) {rend_->keyUp_(key);}
  //! the resize callback
  inline static void resize(int width, int height) {rend_->resize_(width, height);}
  //! the callback for mouse clicks
  inline static void buttonDown(int x, int y, int buttons) {rend_->buttonDown_(x, y, buttons);}
  //! the mouse move callback
  inline static void mouseMove(int x, int y, int buttons) {rend_->mouseMove_(x, y, buttons);}
  //! get the clicked field
  inline Math::Vector2D getClickedField() {return clickedField_;}
  //! reset the clicked field
  inline void resetClickedField() {clickedField_ = Math::Vector2D(-1,-1);}
  //! set the current inventory
  inline void setInventory(Inventory* inv) {inventory_ = inv;}
  //! get the current inventory
  inline Inventory* getInventory() {return inventory_;}
  //! set the view to 3D
  void setViewTo3D(bool threeD);
  //! set awaiting map click
  inline void setAwaitMapclick(void (*proc)(Math::Vector2D click)) {awaitMapClick_ = proc;}
  //! set the trade status
  inline void setTrade(bool on) {trade_ = on;}
protected:
  HQRenderer(Graphics::Renderer* rend);
  void initialize_();
  //! render scene
  void paint_();
  //! resizing scene
  void resize_(int width, int height);
  //! ascii input
  void ascii_(unsigned char key);
  //! special button input
  void special_(int key);
  //! key up callbak
  void keyUp_(int key);
  //! the button clicks
  void buttonDown_(int x, int y, int buttons);
  //! the mouse motion
  void mouseMove_(int x, int y, int buttons);
  //! handle keys continously
  void handleKeys();
  
  //! renderer
  static HQRenderer* rend_;
  //! the engine renderer
  Graphics::Renderer* render_;
  //! ask before quitting app
  bool ask_;
  //! the aspect
  float aspect_;
  //! the field of view
  float fieldOV_;
  //! render 3d
  bool threeD_;
  //! the pressed keys
  bool keys_[256];
  //! the mouse position
  Math::Vector2D mousePos_;
  //! the clicked field
  Math::Vector2D clickedField_;
  //! if map click is awaited
  void (*awaitMapClick_)(Math::Vector2D click);
  //! the inventory that should be rendered
  Inventory* inventory_;
  //! if the trading is active
  bool trade_;
};

#endif
