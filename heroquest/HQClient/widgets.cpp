#include "widgets.h"

#include <cfloat>
#include <cstdio>
#include <sstream>

#include <imgui.h>

#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/forms.h"
#include "renderer/font.h"

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

ImVec4 toColor(const Vector3D& color, float alpha = 1){
  return ImVec4(color.x, color.y, color.z, alpha);
}

ImVec4 brighter(const Vector3D& color, float amount){
  return ImVec4(color.x + amount, color.y + amount, color.z + amount, 1);
}

}

// --- GuiElement ---

GuiElement::GuiElement(){
  pos_ = Vector2D(-1,-1);
  span_ = DEFAULT_SPAN;
  bgColor_ = Vector3D(0.1f, 0.1f, 0.1f);
  fgColor_ = Vector3D(0.0f, 1.0f, 1.0f);
  type_ = UnknownT;
}

float GuiElement::scale(){
  return CGE::Engine::instance()->getVirtualScreen().getScale();
}

bool GuiElement::beginWindow(const Vector2D& pos, const Vector2D& span, bool inputs, int part){
  //the top left corner in window pixels (virtual y is up here)
  const CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  CGE::Vec2i topLeft = screen.toWindow(pos.x, screen.getHeight() - (pos.y + span.y));
  ImGui::SetNextWindowPos(ImVec2((float)topLeft.x, (float)topLeft.y));
  ImGui::SetNextWindowSize(ImVec2(span.x*scale(), span.y*scale()));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1, 1));
  //the text vertically centred in the element
  const float fontSize = FONT_SIZE*scale();
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4*scale(), (span.y*scale() - fontSize)/2));
  ImGui::PushFont(widgets.getFont(), FONT_SIZE*scale());
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
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##text", text_.data(), text_.size(), isHidden_ ? ImGuiInputTextFlags_Password : 0);
    ImGui::PopStyleColor(4);
  }
  endWindow();
  return false;
}

// --- Button ---

Button::Button(){
  handleClicks_ = nullptr;
  setColors(Vector3D(1, 1, 0.1f), Vector3D(0.2f, 0.01f, 0.01f));
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
    clicked = ImGui::Button(getText().c_str(), ImVec2(-FLT_MIN, -FLT_MIN));
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
  setPosition(Vector2D(100, 100));
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
  unsigned maxchars = surSpan_.x/10-1;
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
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  CGE::Color background(0, 0, 0, 0.8f);
  rend->setColor(&background);
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
  forms->activateQuad();
  forms->drawQuad(surPos_, surSpan_);
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
}

bool MessageBox::render(){
  //the message, which doesn't take clicks
  if (beginWindow(surPos_, surSpan_, false, 1)){
    float y = 4;
    for (const string& line : msg_){
      ImGui::SetCursorPos(ImVec2(0, y*scale()));
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", line.c_str());
      y += 20;
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
  CGE::Engine::instance()->getFont(0)->clear();
  CGE::Engine::instance()->getFont(1)->clear();
  CGE::Engine::instance()->getFont(2)->clear();
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
