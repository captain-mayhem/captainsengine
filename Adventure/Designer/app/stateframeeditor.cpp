#include "stateframeeditor.h"
#include "i18n.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <ui/imgui_impl_cge_render.h>

#include "widgets.h"

namespace sd{

namespace{

const int kDragHotspot = 100;
const char* const kFramePayload = "SD_FRAME";
const ImU32 kSelectedFrame = IM_COL32(64, 224, 208, 255);

ImVec2 operator+(ImVec2 a, ImVec2 b) {return ImVec2(a.x+b.x, a.y+b.y);}

//! a dashed rectangle (3 on, 3 off), the frame of an image part
void dashedRect(ImDrawList* list, ImVec2 min, ImVec2 max, ImU32 color){
  const float dash = 3.0f;
  auto line = [&](ImVec2 from, ImVec2 to){
    float len = std::sqrt((to.x-from.x)*(to.x-from.x) + (to.y-from.y)*(to.y-from.y));
    if (len <= 0)
      return;
    ImVec2 dir((to.x-from.x)/len, (to.y-from.y)/len);
    for (float t = 0; t < len; t += 2*dash){
      float end = std::min(t+dash, len);
      list->AddLine(ImVec2(from.x+dir.x*t, from.y+dir.y*t), ImVec2(from.x+dir.x*end, from.y+dir.y*end), color);
    }
  };
  line(min, ImVec2(max.x, min.y));
  line(ImVec2(max.x, min.y), max);
  line(max, ImVec2(min.x, max.y));
  line(ImVec2(min.x, max.y), min);
}

//! the C# editor's crosshair: a circle and four ticks
void crosshair(ImDrawList* list, ImVec2 pos){
  const ImU32 red = IM_COL32(255, 0, 0, 255);
  const float lw = 2.0f, width = 20.0f;
  list->AddCircle(pos, width/2, red, 24, lw);
  list->AddLine(ImVec2(pos.x, pos.y-lw), ImVec2(pos.x, pos.y-lw-width/2-lw), red, lw);
  list->AddLine(ImVec2(pos.x, pos.y+lw), ImVec2(pos.x, pos.y+lw+width/2+lw), red, lw);
  list->AddLine(ImVec2(pos.x-lw, pos.y), ImVec2(pos.x-lw-width/2-lw, pos.y), red, lw);
  list->AddLine(ImVec2(pos.x+lw, pos.y), ImVec2(pos.x+lw+width/2+lw, pos.y), red, lw);
}

Vec2i scaled(Vec2i v, float s) {return Vec2i{(int)(v.x*s), (int)(v.y*s)};}

}

void StateFrameEditor::setState(int state){
  state_ = state;
  frame_ = 0;
  selectedPart_ = -1;
  dragging_ = 0;
  sizeState_ = -1;
}

bool StateFrameEditor::draw(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder){
  if (state_ >= data.states())
    setState(0);
  bool changed = false;
  ImGui::PushID(this);
  changed |= stateButtons(data, style);
  changed |= frameStrip(data, style);
  //the animation steps at the game's speed: a frame every fpsDivider fiftieths of a second
  if (playing_){
    double now = ImGui::GetTime();
    if (now >= nextTick_){
      frame_ = nextFrame(data, state_, frame_);
      nextTick_ = now + data.fpsDivider(state_)/50.0;
    }
  }
  changed |= picture(data, style, images, project, folder);
  changed |= sizeAndSpeed(data, style);
  changed |= popups(data, style);
  ImGui::PopID();
  return changed;
}

bool StateFrameEditor::stateButtons(StateFrames& data, const StateFrameStyle& style){
  int states = data.states();
  auto label = [&](int s){
    return s < (int)style.labels.size() ? toUtf8(style.labels[s]) : std::to_string(s+1);
  };
  if (states <= 10){
    for (int s = 0; s < states; ++s){
      if (s > 0)
        ImGui::SameLine();
      bool current = s == state_;
      if (current)
        ImGui::PushStyleColor(ImGuiCol_Button, kSelectedFrame);
      ImGui::PushID(s);
      if (ImGui::Button(label(s).c_str(), ImVec2(std::max(2.0f*ImGui::GetFontSize(), 0.0f), 0)))
        setState(s);
      ImGui::PopID();
      if (current)
        ImGui::PopStyleColor();
    }
  }
  else{
    ImGui::SetNextItemWidth(16*ImGui::GetFontSize());
    if (ImGui::BeginCombo(tr("State"), label(state_).c_str())){
      for (int s = 0; s < states; ++s){
        ImGui::PushID(s);
        if (ImGui::Selectable(label(s).c_str(), s == state_))
          setState(s);
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }
  return false;
}

bool StateFrameEditor::frameStrip(StateFrames& data, const StateFrameStyle& style){
  const int frames = data.frames();
  const float cell = std::max(std::floor(ImGui::GetContentRegionAvail().x / frames), 10.0f);
  const float height = std::max(cell*1.6f, ImGui::GetFrameHeight());
  ImDrawList* list = ImGui::GetWindowDrawList();
  for (int f = 0; f < frames; ++f){
    if (f > 0)
      ImGui::SameLine(0, 0);
    ImGui::PushID(f);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("frame", ImVec2(cell, height));
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)){
      frame_ = f;
      selectedPart_ = -1;
      if (style.scripts && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
        scriptText_ = toUtf8(data.script(state_, f));
        openScript_ = true;
      }
    }
    if (ImGui::IsItemHovered()){
      std::string tip = "Frame " + std::to_string(f+1);
      for (const std::string& part : data.frame(state_, f)){
        if (!part.empty())
          tip += "\n" + toUtf8(part);
      }
      std::string scr = data.script(state_, f);
      if (!scr.empty())
        tip += "\nScript: " + toUtf8(scr);
      ImGui::SetTooltip("%s", tip.c_str());
    }
    //a frame with images is dragged onto an empty one to copy or move it
    if (!frameEmpty(data, state_, f) && ImGui::BeginDragDropSource()){
      ImGui::SetDragDropPayload(kFramePayload, &f, sizeof(f));
      ImGui::Text("Frame %d", f+1);
      ImGui::EndDragDropSource();
    }
    if (frameEmpty(data, state_, f) && ImGui::BeginDragDropTarget()){
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kFramePayload)){
        fromFrame_ = *(const int*)payload->Data;
        toFrame_ = f;
        openCopyMove_ = true;
      }
      ImGui::EndDragDropTarget();
    }
    ImVec2 min(pos.x, pos.y), max(pos.x+cell-2, pos.y+height-1);
    if (f == frame_)
      list->AddRectFilled(min, max, kSelectedFrame);
    const ImU32 ink = ImGui::GetColorU32(ImGuiCol_Text);
    float w = max.x-min.x;
    if (data.frameExists(state_, f) && !frameEmpty(data, state_, f))
      list->AddCircle(ImVec2(min.x+w/2, min.y+(max.y-min.y)*0.75f), w/4, ink);
    if (!data.script(state_, f).empty())
      list->AddCircle(ImVec2(min.x+w/2, min.y+2+w/4), w/4, ink);
    list->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border));
    ImGui::PopID();
  }
  return false;
}

void StateFrameEditor::drawFrame(ImDrawList* list, StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder, ImVec2 pos, ImVec2 size){
  Vec2i box = scaled(data.size(state_), style.hotspotScale);
  float sx = box.x > 0 ? size.x/box.x : 1, sy = box.y > 0 ? size.y/box.y : 1;
  list->AddRectFilled(pos, pos+size, style.background);
  std::vector<std::string> parts = data.frame(state_, frame_);
  list->PushClipRect(pos, pos+size, true);
  //magnified pixels stay sharp, as in the C# editor's cursor
  const bool magnified = size.x >= 2.0f*data.size(state_).x;
  if (magnified)
    ImGui_ImplCGE_SetSampler(list, images.nearestSampler());
  for (int p = 0; p < (int)parts.size(); ++p){
    if (parts[p].empty())
      continue;
    const ImageCache::Image& img = images.get(project, folder, parts[p]);
    if (!img.texture)
      continue;
    if (style.scaleToBox)
      list->AddImage(img.id(), pos, pos+size);
    else{
      Vec2i off = data.offset(state_, frame_, p);
      ImVec2 a(pos.x+off.x*sx, pos.y+off.y*sy);
      list->AddImage(img.id(), a, ImVec2(a.x+img.width*sx, a.y+img.height*sy));
    }
  }
  if (magnified)
    ImGui_ImplCGE_SetSampler(list, nullptr);
  list->PopClipRect();
}

bool StateFrameEditor::picture(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder){
  bool changed = false;
  Vec2i box = scaled(data.size(state_), style.hotspotScale);
  box.x = std::max(box.x, 1);
  box.y = std::max(box.y, 1);
  ImGui::Spacing();
  //centred, as the C# dialogs place it
  float avail = ImGui::GetContentRegionAvail().x;
  if (avail > box.x)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::floor((avail-box.x)/2));
  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton("picture", ImVec2((float)box.x, (float)box.y), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
  const bool hovered = ImGui::IsItemHovered();
  ImVec2 mousePos = ImGui::GetIO().MousePos;
  Vec2i mouse{(int)(mousePos.x-pos.x), (int)(mousePos.y-pos.y)};
  std::vector<std::string> parts = data.frame(state_, frame_);

  //a press picks the hotspot or a part (the topmost under the mouse)
  bool left = ImGui::IsItemClicked(ImGuiMouseButton_Left), right = ImGui::IsItemClicked(ImGuiMouseButton_Right);
  if (left || right){
    Vec2i center = scaled(data.hotspot(state_), style.hotspotScale);
    float dx = (float)(mouse.x-center.x), dy = (float)(mouse.y-center.y);
    if (style.hotspot && left && std::sqrt(dx*dx+dy*dy) < 10){
      dragOffset_ = Vec2i{mouse.x-center.x, mouse.y-center.y};
      dragging_ = kDragHotspot;
    }
    else{
      selectedPart_ = -1;
      for (int p = (int)parts.size()-1; p >= 0; --p){
        if (parts[p].empty())
          continue;
        const ImageCache::Image& img = images.get(project, folder, parts[p]);
        Vec2i off = style.scaleToBox ? Vec2i{0, 0} : data.offset(state_, frame_, p);
        int w = style.scaleToBox ? box.x : img.width, h = style.scaleToBox ? box.y : img.height;
        if (mouse.x >= off.x && mouse.x <= off.x+w && mouse.y >= off.y && mouse.y <= off.y+h){
          selectedPart_ = p;
          dragOffset_ = Vec2i{mouse.x-off.x, mouse.y-off.y};
          if (left && !style.scaleToBox)
            dragging_ = p+1;
          if (right)
            openRemove_ = true;
          break;
        }
      }
    }
  }
  if (dragging_ != 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
    Vec2i m{std::clamp(mouse.x, 0, box.x), std::clamp(mouse.y, 0, box.y)};
    Vec2i to{m.x-dragOffset_.x, m.y-dragOffset_.y};
    if (dragging_ == kDragHotspot){
      Vec2i spot{(int)(to.x/style.hotspotScale), (int)(to.y/style.hotspotScale)};
      if (!(spot == data.hotspot(state_))){
        data.setHotspot(state_, spot);
        changed = true;
      }
    }
    else if (!(to == data.offset(state_, frame_, dragging_-1))){
      data.setOffset(state_, frame_, dragging_-1, to);
      changed = true;
    }
  }
  else
    dragging_ = 0;
  if (selectedPart_ >= 0 && (ImGui::IsItemFocused() || hovered) && ImGui::IsKeyPressed(ImGuiKey_Delete)){
    removeSelectedPart(data);
    changed = true;
  }

  //images are dropped from the media pool
  if (ImGui::BeginDragDropTarget()){
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kImagePayload)){
      std::string name((const char*)payload->Data, payload->DataSize);
      changed |= dropImage(data, style, images, project, folder, name, mouse);
      parts = data.frame(state_, frame_);
    }
    ImGui::EndDragDropTarget();
  }

  ImDrawList* list = ImGui::GetWindowDrawList();
  drawFrame(list, data, style, images, project, folder, pos, ImVec2((float)box.x, (float)box.y));
  list->PushClipRect(ImVec2(pos.x-2, pos.y-2), ImVec2(pos.x+box.x+2, pos.y+box.y+2), true);
  for (int p = 0; p < (int)parts.size(); ++p){
    if (parts[p].empty())
      continue;
    const ImageCache::Image& img = images.get(project, folder, parts[p]);
    if (!img.texture)
      continue;
    ImU32 color = p == selectedPart_ ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
    if (style.scaleToBox)
      dashedRect(list, ImVec2(pos.x-1, pos.y-1), ImVec2(pos.x+box.x+1, pos.y+box.y+1), color);
    else{
      Vec2i off = data.offset(state_, frame_, p);
      dashedRect(list, ImVec2(pos.x+off.x-1, pos.y+off.y-1), ImVec2(pos.x+off.x+img.width+1, pos.y+off.y+img.height+1), color);
    }
  }
  if (style.hotspot){
    Vec2i spot = scaled(data.hotspot(state_), style.hotspotScale);
    crosshair(list, ImVec2(pos.x+spot.x, pos.y+spot.y));
  }
  list->PopClipRect();

  if (style.imageNames){
    std::string names;
    for (const std::string& part : parts){
      if (part.empty())
        continue;
      names += toUtf8(part) + " ";
      if (!images.get(project, folder, part).texture)
        names += "(not found) ";
    }
    ImGui::TextUnformatted(names.empty() ? "none" : names.c_str());
  }
  return changed;
}

bool StateFrameEditor::dropImage(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder, const std::string& name, Vec2i pos){
  //the part: the next free one, or the one dropped onto (keeping its offset)
  std::vector<std::string> parts = data.frame(state_, frame_);
  int part = 0;
  bool keepOffset = false;
  if (!parts.empty() && (int)parts.size() < data.parts())
    part = (int)parts.size();
  else{
    for (int p = 0; p < (int)parts.size(); ++p){
      if (parts[p].empty()){
        part = p;
        continue;
      }
      const ImageCache::Image& img = images.get(project, folder, parts[p]);
      Vec2i off = data.offset(state_, frame_, p);
      if (pos.x >= off.x && pos.x <= off.x+img.width && pos.y >= off.y && pos.y <= off.y+img.height){
        part = p;
        keepOffset = true;
      }
    }
  }
  if (style.adaptFirstDrop && allFramesEmpty(data)){
    const ImageCache::Image& img = images.get(project, folder, name);
    if (img.texture){
      data.setSize(state_, Vec2i{img.width, img.height});
      sizeState_ = -1;
    }
    pos = Vec2i{0, 0};
  }
  else if (style.adaptFirstDrop && frameEmpty(data, state_, frame_))
    pos = Vec2i{0, 0};
  data.setFramePart(state_, frame_, part, name);
  if (!keepOffset)
    data.setOffset(state_, frame_, part, pos);
  selectedPart_ = part;
  return true;
}

void StateFrameEditor::removeSelectedPart(StateFrames& data){
  if (selectedPart_ < 0)
    return;
  data.setFramePart(state_, frame_, selectedPart_, "");
  selectedPart_ = -1;
}

bool StateFrameEditor::sizeAndSpeed(StateFrames& data, const StateFrameStyle& style){
  bool changed = false;
  const float field = 5*ImGui::GetFontSize();
  if (style.sizeFields){
    if (sizeState_ != state_){
      Vec2i size = data.size(state_);
      width_ = size.x;
      height_ = size.y;
      sizeState_ = state_;
    }
    //taken when the field is left or Enter is pressed, as in the C# editor
    auto apply = [&](){
      width_ = std::clamp(width_, 1, 4096);
      height_ = std::clamp(height_, 1, 4096);
      Vec2i size{width_, height_};
      if (size == data.size(state_))
        return;
      data.setSize(state_, size);
      Vec2i spot = data.hotspot(state_);
      if (style.hotspot && (spot.x > width_ || spot.y > height_))
        data.setHotspot(state_, Vec2i{std::min(spot.x, width_), std::min(spot.y, height_)});
      changed = true;
    };
    ImGui::SetNextItemWidth(field);
    ImGui::InputInt("Width", &width_, 0);
    if (ImGui::IsItemDeactivatedAfterEdit())
      apply();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(field);
    ImGui::InputInt("Height", &height_, 0);
    if (ImGui::IsItemDeactivatedAfterEdit())
      apply();
    if (!ImGui::IsAnyItemActive() && !(data.size(state_) == Vec2i{width_, height_}))
      sizeState_ = -1;
  }
  if (style.animation){
    if (style.sizeFields)
      ImGui::SameLine();
    if (ImGui::Button(playing_ ? "Stop" : "Play", ImVec2(4*ImGui::GetFontSize(), 0))){
      playing_ = !playing_;
      frame_ = 0;
      selectedPart_ = -1;
      nextTick_ = ImGui::GetTime() + data.fpsDivider(state_)/50.0;
    }
  }
  int divider = data.fpsDivider(state_);
  ImGui::SetNextItemWidth(-12*ImGui::GetFontSize());
  if (ImGui::SliderInt("##speed", &divider, 1, 100)){
    data.setFpsDivider(state_, divider);
    changed = true;
  }
  ImGui::SameLine();
  ImGui::Text("Speed %g frames/second", 50.0/std::max(divider, 1));
  return changed;
}

bool StateFrameEditor::popups(StateFrames& data, const StateFrameStyle& style){
  bool changed = false;
  if (openCopyMove_){
    ImGui::OpenPopup(tr("copymove"));
    openCopyMove_ = false;
  }
  if (ImGui::BeginPopup("copymove")){
    if (ImGui::MenuItem(tr("Copy"))){
      copyFrame(data, state_, fromFrame_, toFrame_);
      frame_ = toFrame_;
      changed = true;
    }
    if (ImGui::MenuItem(tr("Move"))){
      moveFrame(data, state_, fromFrame_, toFrame_);
      frame_ = toFrame_;
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (openRemove_){
    ImGui::OpenPopup(tr("remove"));
    openRemove_ = false;
  }
  if (ImGui::BeginPopup("remove")){
    if (ImGui::MenuItem(tr("Remove"))){
      removeSelectedPart(data);
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (openScript_){
    ImGui::OpenPopup(tr("Frame script"));
    openScript_ = false;
  }
  if (ImGui::BeginPopupModal(tr("Frame script"), nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    std::string state = state_ < (int)style.labels.size() ? toUtf8(style.labels[state_]) : std::to_string(state_+1);
    ImGui::Text("Script for %s/%d", state.c_str(), frame_+1);
    if (ImGui::IsWindowAppearing())
      ImGui::SetKeyboardFocusHere();
    ImGui::SetNextItemWidth(30*ImGui::GetFontSize());
    bool enter = inputTextUtf8("##script", scriptText_, ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button(tr("OK"), ImVec2(6*ImGui::GetFontSize(), 0)) || enter){
      std::string text = fromUtf8(scriptText_);
      if (text != data.script(state_, frame_)){
        data.setScript(state_, frame_, text);
        changed = true;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(tr("Cancel"), ImVec2(6*ImGui::GetFontSize(), 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  return changed;
}

}
