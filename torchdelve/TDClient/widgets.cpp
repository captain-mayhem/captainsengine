#include "widgets.h"
#include "layout.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <sstream>

#include <imgui.h>

#include "system/engine.h"
#include "renderer/drawcontext.h"
#include "renderer/forms.h"
#include "renderer/screentext.h"

using std::string;
using std::list;

using namespace Gui;
using CGE::Vector2D;
using CGE::Vector3D;

Gui::Manager widgets;

namespace{

//! the height of the text in virtual units
const float FONT_SIZE = 16;
//! the size of the element's windows in virtual units
const Vector2D DEFAULT_SPAN(150, 18);
//! the height of a message box's text in virtual units
const float MESSAGE_TEXT = 18;

ImVec4 toColor(const Vector3D& color, float alpha = 1){
  return ImVec4(color.x, color.y, color.z, alpha);
}

//! a warmer, brighter shade (hover and press: as if the torchlight caught it)
ImVec4 brighter(const Vector3D& color, float amount){
  return ImVec4(color.x + amount, color.y + amount*0.7f, color.z + amount*0.4f, 1);
}

//! the bronze frame and the rounded corners of buttons, fields and drop-downs (pop with popFrame)
void pushFrame(float scale){
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, (std::max)(1.0f, 1.5f*scale));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 3*scale);
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.42f, 0.2f, 1));
}

void popFrame(){
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(3);
}

}

// --- GuiElement ---

GuiElement::GuiElement(){
  pos_ = Vector2D(-1,-1);
  span_ = DEFAULT_SPAN;
  //dark leather with parchment-coloured text, framed in bronze (pushFrame)
  bgColor_ = Vector3D(0.09f, 0.065f, 0.045f);
  fgColor_ = Vector3D(0.92f, 0.82f, 0.62f);
  type_ = UnknownT;
}

float GuiElement::scale(){
  return CGE::Engine::instance()->getVirtualScreen().getScale();
}

bool GuiElement::beginWindow(const Vector2D& pos, const Vector2D& span, bool inputs, int part, float textHeight){
  //the top left corner in window pixels (virtual y is up here)
  const CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  CGE::Vec2i topLeft = screen.toWindow(pos.x, screen.getHeight() - (pos.y + span.y));
  ImGui::SetNextWindowPos(ImVec2((float)topLeft.x, (float)topLeft.y));
  ImGui::SetNextWindowSize(ImVec2(span.x*scale(), span.y*scale()));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1, 1));
  //the text vertically centred in the element
  //bigger elements (the side panel's buttons) get bigger text, unless the caller sets it
  if (textHeight <= 0)
    textHeight = (std::max)(FONT_SIZE, span.y*0.58f);
  const float fontSize = textHeight*scale();
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4*scale(), (span.y*scale() - fontSize)/2));
  //ImGui scales fonts by the content scale itself, and fontSize is in window pixels already
  ImGui::PushFont(widgets.getFont(), fontSize/ImGui::GetStyle().FontScaleDpi);
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings
    | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus;
  if (!inputs)
    flags |= ImGuiWindowFlags_NoInputs;
  char id[48];
  snprintf(id, sizeof(id), "##widget%p_%d", (void*)this, part);
  return ImGui::Begin(id, nullptr, flags);
}

void GuiElement::endWindow(){
  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(4);
}

// --- InputField ---

InputField::InputField() : text_(256, '\0'){
  isHidden_ = false;
  type_ = InputFieldT;
}

void InputField::setText(const string& text){
  snprintf(text_.data(), text_.size(), "%s", text.c_str());
}

bool InputField::render(){
  if (beginWindow(pos_, span_)){
    ImGui::PushStyleColor(ImGuiCol_FrameBg, toColor(bgColor_));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, brighter(bgColor_, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, brighter(bgColor_, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_Text, toColor(fgColor_));
    pushFrame(scale());
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##text", text_.data(), text_.size(), isHidden_ ? ImGuiInputTextFlags_Password : 0);
    popFrame();
    ImGui::PopStyleColor(4);
  }
  endWindow();
  return false;
}

// --- Button ---

Button::Button(){
  handleClicks_ = nullptr;
  setColors(Vector3D(1.0f, 0.84f, 0.48f), Vector3D(0.2f, 0.12f, 0.06f));
  type_ = ButtonT;
}

bool Button::render(){
  bool clicked = false;
  if (beginWindow(pos_, span_)){
    ImGui::PushStyleColor(ImGuiCol_Button, toColor(bgColor_));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, brighter(bgColor_, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, brighter(bgColor_, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Text, toColor(fgColor_));
    //left aligned: the texts are centred with spaces
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f));
    pushFrame(scale());
    clicked = ImGui::Button(getText().c_str(), ImVec2(-FLT_MIN, -FLT_MIN));
    popFrame();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
  }
  endWindow();
  return clicked;
}

// --- DropDownButton ---

DropDownButton::DropDownButton(){
  setSpan(Vector2D(160, 18));
  selected_ = 0;
  type_ = DropDownButtonT;
}

void DropDownButton::addEntry(const string& entry){
  entries_.push_back(entry);
}

void DropDownButton::clear(){
  entries_.clear();
}

bool DropDownButton::render(){
  bool chosen = false;
  if (beginWindow(pos_, span_)){
    ImGui::PushStyleColor(ImGuiCol_FrameBg, toColor(bgColor_));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, brighter(bgColor_, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, toColor(bgColor_));
    ImGui::PushStyleColor(ImGuiCol_Header, brighter(bgColor_, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, brighter(bgColor_, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Text, toColor(fgColor_));
    pushFrame(scale());
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##entries", getText().c_str(), ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge)){
      for (int i = 0; i < (int)entries_.size(); i++){
        ImGui::PushID(i);
        if (ImGui::Selectable(entries_[i].c_str(), i == selected_)){
          setText(entries_[i]);
          selected_ = i;
          chosen = true;
        }
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
    popFrame();
    ImGui::PopStyleColor(6);
  }
  endWindow();
  return chosen;
}

// --- MessageBox ---

MessageBox::MessageBox(){
  setSpan(Vector2D(75, 18));
  setText("  OK");
  surSpan_ = Vector2D(800, 600);
  setPosition(Layout::dialog(100, 100));
  type_ = MessageBoxT;
}

void MessageBox::process(){
  //close the message box: find yourself
  list<GuiElement*>& elements = widgets.getElements();
  int idx = 0;
  for (list<GuiElement*>::iterator iter = elements.begin(); iter != elements.end(); ++iter){
    if (*iter == this)
      break;
    idx++;
  }
  widgets.remove(idx, false);
  Button::process();
}

void MessageBox::setPosition(Vector2D pos){
  surPos_ = pos;
  //the OK button at the bottom
  Vector2D inpos = pos;
  inpos.x += (short)(0.5*surSpan_.x - 75.0/2.0);
  inpos.y += (short)(0.05*surSpan_.y);
  GuiElement::setPosition(inpos);
}

void MessageBox::setMessage(const string& text){
  std::istringstream str(text);
  //the built-in font is about 0.55 of its height wide per character
  unsigned maxchars = (unsigned)((surSpan_.x - 24)/(MESSAGE_TEXT*0.55f));
  unsigned chars = 0;
  string word;
  string line = "";
  while (str >> word){
    //the word does not fit into the current line
    if (chars + word.size() > maxchars){
      msg_.push_back(line);
      line = "";
      chars = 0;
    }
    //concatenate word to current line
    line += word + " ";
    chars += (unsigned)word.size()+1;
  }
  msg_.push_back(line);
}

void MessageBox::renderBackground(){
  CGE::Gfx::DrawContext& ctx = *CGE::Engine::instance()->getDrawContext();
  CGE::Gfx::BlendMode blend = ctx.getBlend();
  ctx.setBlend(CGE::Gfx::BlendMode::Alpha);
  //dark leather, framed in bronze
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
  ctx.setColor(0.6f, 0.42f, 0.2f, 0.95f);
  forms->drawQuad(ctx, CGE::Vec2f(surPos_.x - 3.0f, surPos_.y - 3.0f), CGE::Vec2f(surSpan_.x + 6.0f, surSpan_.y + 6.0f));
  ctx.setColor(0.07f, 0.05f, 0.035f, 0.92f);
  forms->drawQuad(ctx, surPos_, surSpan_);
  ctx.setColor(1, 1, 1, 1);
  ctx.setBlend(blend);
}

bool MessageBox::render(){
  //the message, which doesn't take clicks; the dialogs' text size (the box is
  //600 high: text from its height would be huge)
  if (beginWindow(surPos_, surSpan_, false, 1, MESSAGE_TEXT)){
    float y = 12;
    for (const string& line : msg_){
      ImGui::SetCursorPos(ImVec2(12*scale(), y*scale()));
      ImGui::TextColored(toColor(Vector3D(0.92f, 0.82f, 0.62f)), "%s", line.c_str());
      y += MESSAGE_TEXT*1.35f;
    }
  }
  endWindow();
  return Button::render();
}

// --- Manager ---

Manager::Manager(){
  clear_ = false;
  font_ = nullptr;
}

Manager::~Manager(){
  for (GuiElement* element : elements_)
    delete element;
  for (GuiElement* element : newElements_)
    delete element;
}

ImFont* Manager::getFont(){
  //ImGui's scalable built-in font, because the elements scale with the window
  if (!font_)
    font_ = ImGui::GetIO().Fonts->AddFontDefaultVector();
  return font_;
}

void Manager::add(GuiElement* element, bool immediate){
  if (immediate){
    elements_.push_back(element);
    return;
  }
  //the first element of a new screen: the old screen's texts go now, so the
  //texts printed for the new one stay
  if (newElements_.empty() && !clear_)
    clearTexts();
  newElements_.push_back(element);
}

void Manager::clear(bool immediate){
  if (!immediate){
    if (newElements_.empty() && !clear_)
      clearTexts();
    clear_ = true;
    return;
  }
  mutex_.lock();
  deleteElements();
  clearTexts();
  mutex_.unlock();
}

void Manager::deleteElements(){
  for (GuiElement* element : elements_)
    delete element;
  elements_.clear();
}

void Manager::clearTexts(){
  CGE::Engine::instance()->getScreenText(0)->clear();
  CGE::Engine::instance()->getScreenText(1)->clear();
  CGE::Engine::instance()->getScreenText(2)->clear();
}

void Manager::remove(int idx, bool immediate){
  if (!immediate){
    removed_.push_back(idx);
    return;
  }
  mutex_.lock();
  if (idx >= 0 && idx < (int)elements_.size()){
    list<GuiElement*>::iterator iter = elements_.begin();
    std::advance(iter, idx);
    delete *iter;
    elements_.erase(iter);
  }
  mutex_.unlock();
}

void Manager::remove(const string& name){
  mutex_.lock();
  for (list<GuiElement*>::iterator iter = elements_.begin(); iter != elements_.end(); ++iter){
    if ((*iter)->getName() == name){
      delete *iter;
      elements_.erase(iter);
      break;
    }
  }
  mutex_.unlock();
}

GuiElement* Manager::get(const string& name){
  for (GuiElement* element : elements_){
    if (element->getName() == name)
      return element;
  }
  return nullptr;
}

void Manager::update(){
  for (int idx : removed_)
    remove(idx);
  removed_.clear();
  if (!newElements_.empty() || clear_){
    clear_ = false;
    //the texts were cleared when the change was requested
    mutex_.lock();
    deleteElements();
    mutex_.unlock();
    elements_ = newElements_;
    newElements_.clear();
  }
}

void Manager::render(){
  update();
  mutex_.lock();
  for (GuiElement* element : elements_)
    element->renderBackground();
  GuiElement* clicked = nullptr;
  for (GuiElement* element : elements_){
    if (element->render() && !clicked)
      clicked = element;
  }
  mutex_.unlock();
  //the callbacks change the elements
  if (clicked)
    clicked->process();
}
