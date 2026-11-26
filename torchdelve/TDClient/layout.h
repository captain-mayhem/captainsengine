//  ==================================================================
// |                         Torchdelve                               |
// |------------------------------------------------------------------|
// |                   A turn-based dungeon crawler                   |
// |                                                                  |
// | Marco Staginski                                                  |
// |------------------------------------------------------------------|
// | File: layout.h                                                   |
//  ==================================================================

#ifndef TD_LAYOUT_H
#define TD_LAYOUT_H

#include <sstream>
#include <string>

#include "math/vector.h"
#include "system/engine.h"
#include "renderer/screentext.h"

//! TDClient's screen layout: a 16:9 virtual screen of 1920 x 1080 (y up for drawing and
//! widgets, as before; clicks come y down).
/*! - The dialogs (connect, login, levels, hero creation, the inventory, the shop, message
      boxes) keep the layout they had on the old 1024 x 768 screen, centred: dialog()
      moves an old position there (y up), dialogClick() moves a click back.
    - The 2D map has square fields on the left; the buttons are in a column in the side
      panel on the right (panelButton()).
    - The 3D view fills the screen. */
namespace Layout{

const int kScreenWidth = 1920;
const int kScreenHeight = 1080;

//! the old 1024 x 768 layout's place: centred
const int kDialogX = (kScreenWidth - 1024)/2;
const int kDialogY = (kScreenHeight - 768)/2;

inline int dialogX(int x) {return x + kDialogX;}
inline int dialogY(int y) {return y + kDialogY;}
inline CGE::Vector2D dialog(int x, int y) {return CGE::Vector2D(dialogX(x), dialogY(y));}
//! a click (y down) in the old layout's coordinates
inline CGE::Vector2D dialogClick(const CGE::Vector2D& click) {return CGE::Vector2D(click.x - kDialogX, click.y - kDialogY);}

//! the side panel on the right
const int kPanelWidth = 440;
const int kPanelX = kScreenWidth - kPanelWidth;
//! the buttons in the panel: kButtonWidth x kButtonHeight
const int kButtonWidth = 240;
const int kButtonHeight = 34;
const int kButtonX = kPanelX + (kPanelWidth - kButtonWidth)/2;
//! a button of the old right column (x 900, y from 30 to 270, 30 or 40 apart) in the panel
inline CGE::Vector2D panelButton(int oldY) {return CGE::Vector2D(kButtonX, 60 + (oldY - 30)*8/5);}
inline CGE::Vector2D panelButtonSpan() {return CGE::Vector2D(kButtonWidth, kButtonHeight);}

//! the 2D map's fields: as big as fit left of the panel and in the height
inline float mapStep(int fieldsX, int fieldsY){
  float sx = (float)(kPanelX - 16)/fieldsX;
  float sy = (float)(kScreenHeight - 16)/fieldsY;
  return (float)(int)(sx < sy ? sx : sy);
}
//! the bottom left corner of the 2D map (y up), centred in its area
inline CGE::Vec2f mapOrigin(int fieldsX, int fieldsY){
  float step = mapStep(fieldsX, fieldsY);
  return CGE::Vec2f((float)(int)((kPanelX - step*fieldsX)/2), (float)(int)((kScreenHeight - step*fieldsY)/2));
}
//! the field under a click (y down); x or y is -1 outside the map
inline CGE::Vector2D mapField(int x, int y, int fieldsX, int fieldsY){
  float step = mapStep(fieldsX, fieldsY);
  CGE::Vec2f origin = mapOrigin(fieldsX, fieldsY);
  float fx = (x - origin.x)/step;
  float fy = (y - (kScreenHeight - origin.y - step*fieldsY))/step;
  CGE::Vector2D field((int)fx, (int)fy);
  if (fx < 0 || field.x >= fieldsX)
    field.x = -1;
  if (fy < 0 || field.y >= fieldsY)
    field.y = -1;
  return field;
}

//! the message line at the top of the screen ("line << text"); the engine's ScreenText puts
//! streamed text at the top of the old 768 high screen, so this prints it itself
struct MessageLine{
  CGE::ScreenText& text() const {return *CGE::Engine::instance()->getScreenText(0);}
  void operator<<(const std::string& msg) const{
    text().clear();
    text().setColor(1.0, 1.0, 1.0);
    text().print(20, kScreenHeight - 26, msg.c_str(), 0, 10);
  }
  void operator<<(const std::ostringstream& msg) const {(*this) << msg.str();}
  void setColor(float r, float g, float b) const {text().setColor(r, g, b);}
  void print(int x, int y, const char* msg, short set, float duration) const {text().print(x, y, msg, set, duration);}
};

}

#endif
