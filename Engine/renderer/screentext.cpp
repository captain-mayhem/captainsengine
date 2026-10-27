#include "screentext.h"

#include "drawcontext.h"
#include "font.h"
#include "renderdevice.h"
#include "spritebatch.h"
#include "../io/Tracing.h"
#include "../platform/assets.h"
#include "../platform/systemfonts.h"

using namespace CGE;

TR_CHANNEL(CGE_ScreenText);

namespace{

//! the texts' size, as the old bitmap font's cells
const float TEXT_SIZE = 16;

}

ScreenText::ScreenText() : show_(true), color_(1, 1, 1, 1), id_(-1){
}

ScreenText::~ScreenText(){
}

void ScreenText::print(int x, int y, const char* str, short set, float duration){
  texts_.push_back({x, y, str ? str : "", set, color_, duration, id_});
  id_ = -1;
}

void ScreenText::setColor(float r, float g, float b){
  color_.r = r;
  color_.g = g;
  color_.b = b;
}

void ScreenText::setColor(const Color& c){
  color_ = c;
}

void ScreenText::render(Gfx::SpriteBatch& batch, float height, double interval){
  TR_USE(CGE_ScreenText);
  if (show_ && !texts_.empty() && !fonts_[0]){
    Gfx::RenderDevice& device = batch.getContext().getDevice();
    std::vector<uint8> data;
    const char* mono = "fonts/LiberationMono-Regular.ttf";
    SystemFonts::FontData system;
    if (!(Assets::exists(mono) && Assets::load(mono, data)) && SystemFonts::find("monospace", system) && system.faceIndex == 0)
      data = std::move(system.data);
    if (data.empty() && !Font::loadDefaultData(data))
      TR_ERROR("no font for the screen texts");
    FontDesc desc;
    desc.size = TEXT_SIZE;
    fonts_[0] = Font::load(device, data, desc);
    desc.embolden = desc.oblique = true;
    fonts_[1] = Font::load(device, std::move(data), desc);
  }
  std::vector<Text> remaining;
  for (Text& text : texts_){
    if (show_ && fonts_[0] && fonts_[1]){
      TextStyle style;
      style.color = text.color;
      //the old cells' top, y down
      fonts_[text.set > 0 ? 1 : 0]->draw(batch, (float)text.x, height - text.y - TEXT_SIZE, text.text, style);
    }
    text.duration -= interval;
    if (text.duration > 0)
      remaining.push_back(std::move(text));
  }
  texts_ = std::move(remaining);
}

void ScreenText::releaseFonts(){
  fonts_[0].reset();
  fonts_[1].reset();
}

void ScreenText::clear(){
  texts_.clear();
}

void ScreenText::operator<<(const std::string& text){
  clear();
  setColor(1.0, 1.0, 1.0);
  print(20, 750, text.c_str(), 0, 10);
}

void ScreenText::operator<<(const std::ostringstream& text){
  (*this) << text.str();
}

void ScreenText::deleteText(short id){
  std::vector<Text> remaining;
  for (Text& text : texts_){
    if (text.id != id)
      remaining.push_back(std::move(text));
  }
  texts_ = std::move(remaining);
}
