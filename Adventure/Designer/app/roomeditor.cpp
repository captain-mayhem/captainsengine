#include "roomeditor.h"
#include "i18n.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "widgets.h"

namespace sd{

namespace{

const ImU32 kRed = IM_COL32(255, 0, 0, 255);
const ImU32 kYellow = IM_COL32(255, 255, 0, 255);
const ImU32 kBlue = IM_COL32(0, 0, 255, 255);
const ImU32 kDarkBlue = IM_COL32(0, 0, 139, 255);
const ImU32 kBlack = IM_COL32(0, 0, 0, 255);
const ImU32 kWhite = IM_COL32(255, 255, 255, 255);
const ImU32 kBrown = IM_COL32(165, 42, 42, 255);
const ImU32 kLightGray = IM_COL32(211, 211, 211, 255);

Vec2i add(Vec2i a, Vec2i b) {return Vec2i{a.x+b.x, a.y+b.y};}
Vec2i sub(Vec2i a, Vec2i b) {return Vec2i{a.x-b.x, a.y-b.y};}
float length(Vec2i v) {return std::sqrt((float)(v.x*v.x + v.y*v.y));}

ImU32 withAlpha(ImU32 color, int alpha){
  return (color & ~IM_COL32_A_MASK) | ((ImU32)alpha << IM_COL32_A_SHIFT);
}

//! text with a black outline, as the C# editor writes into the room
void outlinedText(ImDrawList* list, ImVec2 pos, const std::string& text){
  for (ImVec2 d : {ImVec2(-1, -1), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(1, 1), ImVec2(1, 0), ImVec2(0, 1)})
    list->AddText(ImVec2(pos.x+d.x, pos.y+d.y), kBlack, text.c_str());
  list->AddText(pos, kWhite, text.c_str());
}

const ExtendedFrame* firstFrame(const std::vector<ExtendedFrame>& frames){
  return frames.empty() ? nullptr : &frames[0];
}

//! the directions a character looks, 1 to 4
const char* const kLookDirs[] = {"Down", "Up", "Right", "Left"};

}

bool RoomEditor::selectionValid(const Project& project, const Room& room) const{
  if (!selected_)
    return false;
  if (selected_->character)
    return selected_->index < project.roomCharacters(room.name).size();
  return selected_->index < room.objects.size();
}

bool RoomEditor::draw(Project& project, Room& room, ImageCache& images, const std::string& folder, Requests& requests){
  bool changed = false;
  if (!selectionValid(project, room))
    selected_.reset();
  int mode = (int)mode_;
  if (radioButtons(mode, {"Objects", "Walkmap", "Deepmap", "Inventory field", "FX shapes"}, true)){
    mode_ = (RoomMode)mode;
    drag_ = Drag::None;
  }
  ImGui::SameLine();
  ImGui::TextDisabled("%s", tr("(keys 1 to 5)"));
  const float panelWidth = 19*ImGui::GetFontSize();
  const float canvasWidth = std::max(ImGui::GetContentRegionAvail().x - panelWidth - ImGui::GetStyle().ItemSpacing.x, 50.0f);
  if (ImGui::BeginChild("canvas", ImVec2(canvasWidth, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    changed |= canvas(project, room, images, folder, requests);
  ImGui::EndChild();
  ImGui::SameLine();
  if (ImGui::BeginChild("panel", ImVec2(0, 0)))
    changed |= panel(project, room, requests);
  ImGui::EndChild();
  return changed;
}

bool RoomEditor::canvas(Project& project, Room& room, ImageCache& images, const std::string& folder, Requests& requests){
  bool changed = false;
  const Vec2i view{windowWidth(project.settings), project.settings.resolution.y};
  if (view.x <= 0 || view.y <= 0 || project.walkGridSize() == 0){
    ImGui::TextDisabled("The project's resolution has no room.");
    return false;
  }
  ImVec2 avail = ImGui::GetContentRegionAvail();
  scale_ = std::min(avail.x/view.x, avail.y/view.y);
  if (scale_ <= 0.05f)
    scale_ = 0.05f;
  const ImVec2 size(view.x*scale_, view.y*scale_);
  origin_ = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton("room", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
  const bool hovered = ImGui::IsItemHovered();
  if (mode_ == RoomMode::Objects)
    changed |= drop(project, room);
  const ImVec2 mouse = ImGui::GetIO().MousePos;
  if (hovered || drag_ != Drag::None)
    mouse_ = Vec2i{(int)std::floor((mouse.x-origin_.x)/scale_), (int)std::floor((mouse.y-origin_.y)/scale_)};

  if (mode_ == RoomMode::Inventory && room.hasInventory && (hovered || drag_ != Drag::None))
    inventoryCursor(room);
  if (hovered){
    for (ImGuiMouseButton button : {ImGuiMouseButton_Left, ImGuiMouseButton_Right, ImGuiMouseButton_Middle}){
      if (ImGui::IsMouseClicked(button))
        changed |= mouseDown(project, room, button);
    }
    if (mode_ == RoomMode::Walkmap && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
      Vec2i field = walkmapField(project, room, add(mouse_, room.scrollOffset));
      if (inWalkmap(room, field)){
        walkmapScript(project, room, field);
        requests.openScript = true;
        requests.scriptType = Script::Walkmap;
        requests.script = Script::walkmapScriptName(field.x, field.y, room.name);
        changed = true;
      }
    }
  }
  if (drag_ != Drag::None){
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      changed |= mouseDrag(project, room);
    else
      drag_ = Drag::None;
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput){
    const ImGuiKey keys[] = {ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5};
    for (int i = 0; i < 5; ++i){
      if (ImGui::IsKeyPressed(keys[i], false)){
        mode_ = (RoomMode)i;
        drag_ = Drag::None;
      }
    }
  }

  ImDrawList* list = ImGui::GetWindowDrawList();
  list->PushClipRect(origin_, ImVec2(origin_.x+size.x, origin_.y+size.y), true);
  list->AddRectFilled(origin_, ImVec2(origin_.x+size.x, origin_.y+size.y), IM_COL32(40, 40, 40, 255));
  drawRoom(list, project, room, images, folder);
  drawOverlay(list, project, room);
  list->PopClipRect();
  changed |= popups(project, room, requests);
  return changed;
}

void RoomEditor::drawRoom(ImDrawList* list, Project& project, Room& room, ImageCache& images, const std::string& folder){
  const ImU32 tint = IM_COL32(room.lighting[0], room.lighting[1], room.lighting[2], 255);
  const Vec2i scroll = room.scrollOffset;
  auto screen = [this, scroll](float x, float y){
    return ImVec2(origin_.x + (x-scroll.x)*scale_, origin_.y + (y-scroll.y)*scale_);
  };
  auto image = [&](const std::string& name){
    const ImageCache::Image& img = images.get(project, folder, name);
    if (img.texture)
      list->AddImage(img.id(), screen(0, 0), screen((float)img.width, (float)img.height), ImVec2(0, 0), ImVec2(1, 1), tint);
  };
  if (!room.parallaxBackground.empty())
    image(room.parallaxBackground);
  if (!room.background.empty())
    image(room.background);

  // a frame's parts in a picture of the state's size (which cuts them), at a
  // room position, scaled and maybe mirrored
  auto frame = [&](const ExtendedFrame* f, Vec2i box, float x, float y, float scale, bool flip){
    if (f == nullptr || box.x <= 0 || box.y <= 0)
      return;
    for (size_t p = 0; p < f->names.size(); ++p){
      if (f->names[p].empty())
        continue;
      const ImageCache::Image& img = images.get(project, folder, f->names[p]);
      if (!img.texture || img.width <= 0 || img.height <= 0)
        continue;
      Vec2i off = p < f->offsets.size() ? f->offsets[p] : Vec2i{};
      float x0 = (float)std::max(off.x, 0), x1 = (float)std::min(off.x+img.width, box.x);
      float y0 = (float)std::max(off.y, 0), y1 = (float)std::min(off.y+img.height, box.y);
      if (x0 >= x1 || y0 >= y1)
        continue;
      ImVec2 uv0((x0-off.x)/img.width, (y0-off.y)/img.height);
      ImVec2 uv1((x1-off.x)/img.width, (y1-off.y)/img.height);
      if (flip){
        float fx0 = box.x - x1, fx1 = box.x - x0;
        x0 = fx0;
        x1 = fx1;
        std::swap(uv0.x, uv1.x);
      }
      list->AddImage(img.id(), screen(x+x0*scale, y+y0*scale), screen(x+x1*scale, y+y1*scale), uv0, uv1, tint);
    }
  };
  // the corners of an object or character, as the C# editor marks them
  auto boundary = [&](Vec2i pos, Vec2i box, ImU32 color){
    const float span = 4;
    float x = (float)pos.x, y = (float)pos.y, w = (float)box.x, h = (float)box.y;
    auto line = [&](float ax, float ay, float bx, float by){list->AddLine(screen(ax, ay), screen(bx, by), color);};
    line(x-1, y-1, x+span, y-1);
    line(x-1, y-1, x-1, y+span);
    line(x+w, y-1, x+w-span-1, y-1);
    line(x+w, y-1, x+w, y+span);
    line(x-1, y+h, x+span, y+h);
    line(x-1, y+h, x-1, y+h-span-1);
    line(x+w, y+h, x+w-span-1, y+h);
    line(x+w, y+h, x+w, y+h-span-1);
  };
  const int grid = project.walkGridSize();
  const bool marks = mode_ == RoomMode::Objects;
  // the selection blinks red and yellow, a quarter second each
  const ImU32 selectedColor = ((int)(ImGui::GetTime()/0.05) % 10) < 5 ? kYellow : kRed;
  const std::vector<CharacterInstance>& chars = project.roomCharacters(room.name);
  for (const RoomEntry& entry : drawOrder(project, room)){
    const bool isSelected = selected_ && *selected_ == entry;
    const ImU32 color = isSelected ? selectedColor : kRed;
    if (entry.character){
      const CharacterInstance& inst = chars[entry.index];
      const Character* chr = project.characters.find(inst.character);
      int state = characterState(inst);
      if (chr == nullptr || state >= (int)chr->states.size())
        continue;
      const CharacterState& cs = chr->states[state];
      float scale = roomScale(project, room, characterFoot(project, inst));
      if (chr->noZoom)
        scale = 1.0f;
      scale *= chr->zoom/100.0f;
      const Vec2i spot = cs.basepoint;
      Vec2i render{inst.position.x + spot.x - (int)(spot.x*scale), inst.position.y + spot.y - (int)(spot.y*scale)};
      const bool flip = inst.lookDir == 4;
      if (flip)
        render.x += (int)((-cs.size.x + 2*spot.x)*scale);
      frame(firstFrame(cs.frames), cs.size, (float)render.x, (float)render.y, scale, flip);
      if (marks)
        boundary(inst.position, cs.size, color);
    }
    else{
      const ObjectInstance& inst = room.objects[entry.index];
      const Object* obj = project.objects.find(inst.object);
      if (obj == nullptr)
        continue;
      if (inst.state >= 1 && inst.state <= (int)obj->states.size())
        frame(firstFrame(obj->states[inst.state-1].frames), obj->size, (float)inst.position.x, (float)inst.position.y, 1.0f, false);
      if (marks){
        boundary(inst.position, obj->size, color);
        if (inst.layer == 1 && grid > 0){
          float yc = (float)(int)(inst.depth*grid/2 - grid/4);
          float x = (float)inst.position.x;
          list->AddLine(screen(x, (float)inst.position.y), screen(x, yc), kBlue);
          list->AddRectFilled(screen(x-2, yc-2), screen(x+3, yc+3), kBlue);
          list->AddRect(screen(x-5, yc-5), screen(x+5, yc+5), kBlue);
        }
      }
    }
  }
}

void RoomEditor::drawOverlay(ImDrawList* list, const Project& project, const Room& room){
  const Vec2i scroll = room.scrollOffset;
  const int grid = project.walkGridSize();
  auto screen = [this, scroll](float x, float y){
    return ImVec2(origin_.x + (x-scroll.x)*scale_, origin_.y + (y-scroll.y)*scale_);
  };
  const Vec2i rp = add(mouse_, scroll);
  const float lineHeight = ImGui::GetTextLineHeight();
  char buf[256];
  const int wmscale = walkmapScale(room);
  Vec2i field = walkmapField(project, room, rp);
  if (!room.walkmap.empty()){
    field.x = std::clamp(field.x, 0, (int)room.walkmap.size()-1);
    field.y = std::clamp(field.y, 0, std::max((int)room.walkmap[field.x].size()-1, 0));
  }
  const bool wmfree = inWalkmap(room, field) && room.walkmap[field.x][field.y].isFree;

  switch (mode_){
    case RoomMode::Objects:{
      if (project.settings.infoLine){
        snprintf(buf, sizeof(buf), "Position: Room(%d/%d) Walkmap(%d/%d)%c Mouse(%d/%d)", scroll.x/grid, scroll.y/grid, field.x+1, field.y+1, wmfree ? 'F' : 'B', rp.x, rp.y);
        outlinedText(list, origin_, buf);
      }
      std::string line = "Object:";
      if (std::optional<RoomEntry> entry = entryAt(project, room, rp, false)){
        if (entry->character){
          const CharacterInstance& chr = project.roomCharacters(room.name)[entry->index];
          Vec2i foot = characterFoot(project, chr);
          snprintf(buf, sizeof(buf), " (WalkmapPos: %d,%d)", wmscale*foot.x/grid+1, wmscale*foot.y/grid+1);
          line = "Object: " + toUtf8(chr.name) + buf;
        }
        else{
          const ObjectInstance& obj = room.objects[entry->index];
          std::string depth;
          if (obj.layer == 0)
            depth = "Back";
          else if (obj.layer == 1)
            depth = std::to_string(obj.depth/2) + "/" + std::to_string(obj.depth);
          else
            depth = "Front";
          snprintf(buf, sizeof(buf), " (PixelPos: %d,%d Depth: ", obj.position.x, obj.position.y);
          line = "Object: " + toUtf8(obj.name) + buf + depth + ")";
        }
      }
      outlinedText(list, ImVec2(origin_.x, origin_.y+lineHeight), line);
      break;
    }
    case RoomMode::Walkmap:{
      const float s = room.doubleWalkmap ? 0.5f : 1.0f;
      const float cell = grid*s;
      const ImU32 blocked = IM_COL32(0, 0, 255, 100);
      for (int x = 0; x < (int)room.walkmap.size(); ++x){
        for (int y = 0; y < (int)room.walkmap[x].size(); ++y){
          const WalkMapEntry& e = room.walkmap[x][y];
          if (!e.isFree)
            list->AddRectFilled(screen(x*cell, y*cell), screen((x+1)*cell, (y+1)*cell), blocked);
          if (e.hasScript){
            float x0 = (x*grid + 4)*s, y0 = (y*grid + 4)*s, w = (grid - 8)*s;
            list->AddRect(screen(x0, y0), screen(x0+w, y0+w), kBlack);
            list->AddRect(screen(x0+1, y0+1), screen(x0+1+w, y0+1+w), kYellow);
          }
        }
      }
      if (cell > 0){
        for (float i = cell; i < room.size.x; i += cell)
          list->AddLine(screen(i, 0), screen(i, (float)room.size.y), kBrown);
        for (float i = cell; i < room.size.y; i += cell)
          list->AddLine(screen(0, i), screen((float)room.size.x, i), kBrown);
      }
      const int pen = (int)pen_;
      if (pen < 3){
        const int size = 2*pen + 1;
        float cx = ((field.x - pen)*grid + size*grid/2.0f)*s;
        float cy = ((field.y - pen)*grid + size*grid/2.0f)*s;
        list->AddCircle(screen(cx, cy), (size*grid - 4)/2.0f*s*scale_, kBlack);
        list->AddCircle(screen(cx, cy), (size*grid - 2)/2.0f*s*scale_, kLightGray);
      }
      else{
        ImU32 color = pen == 3 ? IM_COL32(255, 255, 0, 100) : IM_COL32(255, 0, 0, 100);
        list->AddRectFilled(screen(field.x*cell, field.y*cell), screen((field.x+1)*cell, (field.y+1)*cell), color);
      }
      snprintf(buf, sizeof(buf), "%s (%d,%d)", wmfree ? "Free" : "Blocked", field.x+1, field.y+1);
      outlinedText(list, origin_, buf);
      break;
    }
    case RoomMode::Deepmap:{
      const float y1 = (float)(room.depthmap.x*grid);
      const float y2 = (float)(room.depthmap.y*grid);
      const float right = (float)std::max(room.size.x, project.settings.resolution.x*3);
      list->AddRectFilled(screen(0, 0), screen(right, y1), IM_COL32(0, 0, 255, 100));
      list->AddRectFilled(screen(0, y1), screen(right, y2), IM_COL32(0, 0, 255, 40));
      list->AddLine(screen(0, y1), screen(right, y1), kBlue, 3.0f);
      list->AddLine(screen(0, y2), screen(right, y2), kBlue, 3.0f);
      ImVec2 p1 = screen((float)scroll.x, y1);
      ImVec2 p2 = screen((float)scroll.x, y2);
      outlinedText(list, ImVec2(p1.x, p1.y-lineHeight), "Depth 1: " + std::to_string(room.depthmap.x) + " (Pull me)");
      outlinedText(list, ImVec2(p2.x, p2.y-lineHeight), "Depth 2: " + std::to_string(room.depthmap.y) + " (Pull me)");
      break;
    }
    case RoomMode::Inventory:{
      if (!room.hasInventory){
        outlinedText(list, origin_, "Draw an inventoryfield into the room.");
        break;
      }
      const Vec2i pix = inventoryPixels(room);
      const float px = (float)room.invPos.x, py = (float)room.invPos.y;
      list->AddRectFilled(screen(px, py), screen(px+pix.x, py+pix.y), IM_COL32(0, 0, 255, 50));
      list->AddRect(screen(px, py), screen(px+pix.x, py+pix.y), kDarkBlue);
      for (int x = 0; x < room.invSize.x; ++x){
        for (int y = 0; y < room.invSize.y; ++y){
          float cx = (float)(x*((int)(50*room.invScale.x) + room.invSpacing) + 30) + px;
          float cy = (float)(y*((int)(50*room.invScale.y) + room.invSpacing) + 5) + py;
          list->AddRect(screen(cx, cy), screen(cx + 50*room.invScale.x, cy + 48*room.invScale.y), kBlue);
        }
      }
      const float ydown = py + pix.y, xmiddle = px + pix.x/2, ymiddle = py + pix.y/2, xright = px + pix.x;
      list->AddLine(screen(xmiddle-15, ydown-3), screen(xmiddle+15, ydown-3), kDarkBlue, 5*scale_);
      list->AddLine(screen(xright-3, ymiddle-15), screen(xright-3, ymiddle+15), kDarkBlue, 5*scale_);
      ImVec2 corner[4] = {screen(xright-11, ydown), screen(xright-5, ydown), screen(xright, ydown-5), screen(xright, ydown-11)};
      list->AddConvexPolyFilled(corner, 4, kDarkBlue);
      snprintf(buf, sizeof(buf), "Inventory field: %dx%d fields", room.invSize.x, room.invSize.y);
      outlinedText(list, origin_, buf);
      break;
    }
    case RoomMode::SpecialFx:{
      const ImU32 colors[] = {kBlue, IM_COL32(0, 128, 0, 255), kRed};
      for (size_t i = 0; i < room.fxShapes.size() && i < 3; ++i){
        const FxShape& shape = room.fxShapes[i];
        if (!shape.active)
          continue;
        ImVec2 points[4];
        for (int p = 0; p < 4; ++p)
          points[p] = screen((float)shape.positions[p].x, (float)shape.positions[p].y);
        list->AddConcavePolyFilled(points, 4, withAlpha(colors[i], 75));
        list->AddPolyline(points, 4, colors[i], ImDrawFlags_Closed, 1.0f);
        for (int p = 0; p < 4; ++p)
          list->AddCircle(points[p], 4*scale_, colors[i]);
        Vec2i center = fxCenter(shape);
        ImVec2 c = screen((float)center.x, (float)center.y);
        const float r = 10*scale_;
        list->AddCircle(c, r, colors[i]);
        list->AddLine(ImVec2(c.x, c.y-1), ImVec2(c.x, c.y-2-r), colors[i]);
        list->AddLine(ImVec2(c.x, c.y+1), ImVec2(c.x, c.y+2+r), colors[i]);
        list->AddLine(ImVec2(c.x-1, c.y), ImVec2(c.x-2-r, c.y), colors[i]);
        list->AddLine(ImVec2(c.x+1, c.y), ImVec2(c.x+2+r, c.y), colors[i]);
        const char* label = "";
        if (shape.effect == FxShape::WallMirror){
          // the handle where it's taken (the C# editor draws it half as far down)
          float yc = (float)fxDepthHandleY(project, shape);
          float x = (float)center.x;
          list->AddLine(c, screen(x, yc), colors[i]);
          list->AddRectFilled(screen(x-2, yc-2), screen(x+3, yc+3), colors[i]);
          list->AddRect(screen(x-5, yc-5), screen(x+5, yc+5), colors[i]);
          label = "Wall-Mirror";
        }
        else if (shape.effect == FxShape::FloorMirror)
          label = "Floor-Mirror";
        else if (shape.effect == FxShape::ParticleBarrier)
          label = "Particle-Barrier";
        outlinedText(list, points[0], label);
      }
      break;
    }
  }
}

namespace{

//! the inventory field's handle at a room position
enum class InvHandle{None, Bottom, Right, Corner, Inside};

InvHandle inventoryHandle(const Room& room, Vec2i p){
  const Vec2i pix = inventoryPixels(room);
  const int ydown = room.invPos.y + pix.y;
  const int xmiddle = room.invPos.x + pix.x/2;
  const int ymiddle = room.invPos.y + pix.y/2;
  const int xright = room.invPos.x + pix.x;
  InvHandle handle = InvHandle::None;
  // as the C# editor tests them: the middle column only has the bottom handle
  if (p.x >= xmiddle-15 && p.x <= xmiddle+15){
    if (p.y <= ydown+3 && p.y >= ydown-5-3)
      handle = InvHandle::Bottom;
  }
  else if (p.y >= ymiddle-15 && p.y <= ymiddle+15){
    if (p.x <= xright+3 && p.x >= xright-5-3)
      handle = InvHandle::Right;
  }
  else if (p.x >= xright-11-3 && p.x <= xright+3){
    if (p.y >= ydown-11-3 && p.y <= ydown+3)
      handle = InvHandle::Corner;
  }
  if (handle == InvHandle::None && p.x >= room.invPos.x && p.x <= xright && p.y >= room.invPos.y && p.y <= ydown)
    handle = InvHandle::Inside;
  return handle;
}

}

void RoomEditor::inventoryCursor(const Room& room) const{
  InvHandle handle = InvHandle::None;
  switch (drag_){
    case Drag::InvBottom: handle = InvHandle::Bottom; break;
    case Drag::InvRight: handle = InvHandle::Right; break;
    case Drag::InvCorner: handle = InvHandle::Corner; break;
    case Drag::None: handle = inventoryHandle(room, add(mouse_, room.scrollOffset)); break;
    default: break;
  }
  if (handle == InvHandle::Bottom)
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
  else if (handle == InvHandle::Right)
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  else if (handle == InvHandle::Corner)
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
}

bool RoomEditor::mouseDown(Project& project, Room& room, ImGuiMouseButton button){
  const Vec2i pos = mouse_;
  const Vec2i rp = add(pos, room.scrollOffset);
  const int grid = project.walkGridSize();
  switch (mode_){
    case RoomMode::Objects:{
      if (button == ImGuiMouseButton_Left || button == ImGuiMouseButton_Middle){
        for (size_t i = 0; i < room.objects.size(); ++i){
          const ObjectInstance& obj = room.objects[i];
          if (obj.layer != 1)
            continue;
          Vec2i center{obj.position.x, obj.depth*grid/2 - grid/4};
          if (length(sub(center, rp)) <= 5){
            selected_ = RoomEntry{false, i, objectDepth(obj)};
            dragOffset_ = sub(center, pos);
            drag_ = Drag::ObjectDepth;
            return false;
          }
        }
      }
      selected_ = entryAt(project, room, rp, button == ImGuiMouseButton_Middle);
      if (selected_){
        Vec2i at = selected_->character ? characterFoot(project, project.roomCharacters(room.name)[selected_->index]) : room.objects[selected_->index].position;
        dragOffset_ = sub(at, pos);
        drag_ = button == ImGuiMouseButton_Right ? Drag::None : Drag::Entry;
      }
      if (button == ImGuiMouseButton_Right){
        if (selected_)
          openObjectMenu_ = true;
        else
          openRoomMenu_ = true;
      }
      return false;
    }
    case RoomMode::Walkmap:{
      Vec2i field = walkmapField(project, room, rp);
      if (button == ImGuiMouseButton_Right){
        menuField_ = field;
        openWalkmapMenu_ = true;
        return false;
      }
      drag_ = Drag::Walkmap;
      return modifyWalkmap(project, room, field);
    }
    case RoomMode::Deepmap:{
      int depth = depthmapLineAt(project, room, rp.y);
      if (depth == room.depthmap.x)
        drag_ = Drag::DepthTop;
      else if (depth == room.depthmap.y)
        drag_ = Drag::DepthBottom;
      return false;
    }
    case RoomMode::Inventory:{
      if (room.hasInventory){
        if (button == ImGuiMouseButton_Right){
          openInventoryMenu_ = true;
          return false;
        }
        const Vec2i pix = inventoryPixels(room);
        switch (inventoryHandle(room, rp)){
          case InvHandle::Bottom:
            drag_ = Drag::InvBottom;
            dragOffset_.y = rp.y - (room.invPos.y + pix.y);
            break;
          case InvHandle::Right:
            drag_ = Drag::InvRight;
            dragOffset_.x = rp.x - (room.invPos.x + pix.x);
            break;
          case InvHandle::Corner:
            drag_ = Drag::InvCorner;
            break;
          case InvHandle::Inside:
            drag_ = Drag::InvPosition;
            dragOffset_ = sub(rp, room.invPos);
            break;
          case InvHandle::None:
            break;
        }
        return false;
      }
      if (button == ImGuiMouseButton_Right)
        return false;
      room.invPos = rp;
      room.invScale = Vec2f{1.0f, 1.0f};
      room.hasInventory = true;
      drag_ = Drag::InvCorner;
      return true;
    }
    case RoomMode::SpecialFx:{
      if (button == ImGuiMouseButton_Right)
        return false;
      for (size_t idx = 0; idx < room.fxShapes.size(); ++idx){
        const FxShape& shape = room.fxShapes[idx];
        if (!shape.active)
          continue;
        for (int i = 0; i < 4; ++i){
          if (length(sub(shape.positions[i], rp)) <= 4){
            drag_ = Drag::FxCorner;
            fxShape_ = (int)idx;
            fxCorner_ = i;
            return false;
          }
        }
        Vec2i center = fxCenter(shape);
        if (length(sub(center, rp)) <= 10){
          drag_ = Drag::FxPosition;
          fxShape_ = (int)idx;
          dragOffset_ = pos;
          return false;
        }
        if (shape.effect == FxShape::WallMirror){
          Vec2i handle{center.x, fxDepthHandleY(project, shape)};
          if (length(sub(handle, rp)) <= 5){
            drag_ = Drag::FxDepth;
            fxShape_ = (int)idx;
            dragOffset_ = sub(handle, pos);
            return false;
          }
        }
      }
      return false;
    }
  }
  return false;
}

bool RoomEditor::mouseDrag(Project& project, Room& room){
  const Vec2i rp = add(mouse_, room.scrollOffset);
  switch (drag_){
    case Drag::None:
      return false;
    case Drag::Entry:{
      if (!selectionValid(project, room))
        return false;
      if (selected_->character){
        std::vector<CharacterInstance>* chars = project.characterInstances.find(room.name);
        CharacterInstance& chr = (*chars)[selected_->index];
        if (chr.locked)
          return false;
        Vec2i before = chr.position;
        setCharacterFoot(project, chr, add(mouse_, dragOffset_));
        return !(before == chr.position);
      }
      ObjectInstance& obj = room.objects[selected_->index];
      if (obj.locked)
        return false;
      Vec2i before = obj.position;
      obj.position = add(mouse_, dragOffset_);
      return !(before == obj.position);
    }
    case Drag::ObjectDepth:{
      if (!selectionValid(project, room) || selected_->character)
        return false;
      ObjectInstance& obj = room.objects[selected_->index];
      int before = obj.depth;
      obj.depth = objectDepthAt(project, room, mouse_.y + dragOffset_.y);
      return before != obj.depth;
    }
    case Drag::Walkmap:
      return modifyWalkmap(project, room, walkmapField(project, room, rp));
    case Drag::DepthTop:
    case Drag::DepthBottom:{
      Vec2i before = room.depthmap;
      setDepthmapLine(project, room, drag_ == Drag::DepthTop ? 0 : 1, depthmapLineAt(project, room, rp.y));
      return !(before == room.depthmap);
    }
    case Drag::InvCorner:{
      Vec2i before = room.invSize;
      dragInventoryCorner(project, room, rp);
      return !(before == room.invSize);
    }
    case Drag::InvPosition:{
      Vec2i before = room.invPos;
      room.invPos = sub(rp, dragOffset_);
      room.invPos.x = std::max(room.invPos.x, 0);
      room.invPos.y = std::max(room.invPos.y, 0);
      return !(before == room.invPos);
    }
    case Drag::InvBottom:
    case Drag::InvRight:{
      Vec2f before = room.invScale;
      if (drag_ == Drag::InvBottom)
        scaleInventory(room, 1, (float)(rp.y - dragOffset_.y - room.invPos.y));
      else
        scaleInventory(room, 0, (float)(rp.x - dragOffset_.x - room.invPos.x));
      return before.x != room.invScale.x || before.y != room.invScale.y;
    }
    case Drag::FxCorner:
    case Drag::FxPosition:
    case Drag::FxDepth:{
      if (fxShape_ < 0 || fxShape_ >= (int)room.fxShapes.size())
        return false;
      FxShape& shape = room.fxShapes[fxShape_];
      if (drag_ == Drag::FxCorner){
        Vec2i before = shape.positions[fxCorner_];
        setFxCorner(room, shape, fxCorner_, rp);
        return !(before == shape.positions[fxCorner_]);
      }
      if (drag_ == Drag::FxPosition){
        Vec2i diff = sub(mouse_, dragOffset_);
        dragOffset_ = mouse_;
        if (diff.x == 0 && diff.y == 0)
          return false;
        moveFxShape(room, shape, diff);
        return true;
      }
      int before = shape.depth;
      shape.depth = fxDepthAt(project, room, mouse_.y + dragOffset_.y);
      return before != shape.depth;
    }
  }
  return false;
}

bool RoomEditor::modifyWalkmap(Project& project, Room& room, Vec2i field){
  if (!inWalkmap(room, field))
    return false;
  WalkMapEntry& e = room.walkmap[field.x][field.y];
  switch (pen_){
    case WalkmapPen::Size1:
    case WalkmapPen::Size2:
    case WalkmapPen::Size3:{
      paintWalkmap(room, field, pen_, penFree_);
      return true;
    }
    case WalkmapPen::ScriptPaster:
      if (!scriptCopied_)
        return false;
      pasteWalkmapScript(project, room, field, copiedScript_);
      return true;
    case WalkmapPen::ScriptEraser:
      if (!e.hasScript)
        return false;
      eraseWalkmapScript(project, room, field);
      return true;
  }
  return false;
}

bool RoomEditor::drop(Project& project, Room& room){
  if (!ImGui::BeginDragDropTarget())
    return false;
  bool changed = false;
  const ImVec2 mouse = ImGui::GetIO().MousePos;
  const Vec2i rp{(int)std::floor((mouse.x-origin_.x)/scale_) + room.scrollOffset.x, (int)std::floor((mouse.y-origin_.y)/scale_) + room.scrollOffset.y};
  if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kObjectPayload)){
    std::string name((const char*)payload->Data, payload->DataSize);
    if (project.objects.find(name)){
      placeObject(project, room, name, rp);
      selected_ = RoomEntry{false, room.objects.size()-1, objectDepth(room.objects.back())};
      changed = true;
    }
  }
  if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kCharacterPayload)){
    std::string name((const char*)payload->Data, payload->DataSize);
    if (project.characters.find(name)){
      CharacterInstance& chr = placeCharacter(project, room.name, name, rp);
      selected_ = RoomEntry{true, project.roomCharacters(room.name).size()-1, characterDepth(project, chr)};
      changed = true;
    }
  }
  if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SD_IMAGE")){
    pendingImage_.assign((const char*)payload->Data, payload->DataSize);
    openImageMenu_ = true;
  }
  ImGui::EndDragDropTarget();
  return changed;
}

bool RoomEditor::popups(Project& project, Room& room, Requests& requests){
  bool changed = false;
  if (openImageMenu_){
    ImGui::OpenPopup(tr("image"));
    openImageMenu_ = false;
  }
  if (openObjectMenu_){
    ImGui::OpenPopup(tr("object"));
    openObjectMenu_ = false;
  }
  if (openRoomMenu_){
    ImGui::OpenPopup(tr("background"));
    openRoomMenu_ = false;
  }
  if (openWalkmapMenu_){
    ImGui::OpenPopup(tr("walkmap"));
    openWalkmapMenu_ = false;
  }
  if (openInventoryMenu_){
    ImGui::OpenPopup(tr("inventory"));
    openInventoryMenu_ = false;
  }
  if (ImGui::BeginPopup("image")){
    if (ImGui::MenuItem(tr("Add as Background"))){
      room.background = pendingImage_;
      changed = true;
    }
    if (ImGui::MenuItem(tr("Add as Parallax Background"))){
      room.parallaxBackground = pendingImage_;
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (ImGui::BeginPopup("background")){
    if (ImGui::MenuItem(tr("Remove Background"), nullptr, false, !room.background.empty())){
      room.background.clear();
      changed = true;
    }
    if (ImGui::MenuItem(tr("Remove Parallax Background"), nullptr, false, !room.parallaxBackground.empty())){
      room.parallaxBackground.clear();
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (ImGui::BeginPopup("object")){
    const bool valid = selectionValid(project, room);
    const bool character = valid && selected_->character;
    if (ImGui::MenuItem(tr("Remove"), nullptr, false, valid)){
      if (project.settings.silentDelete){
        if (character)
          removeCharacterInstance(project, room.name, selected_->index);
        else
          removeObjectInstance(project, room, selected_->index);
        selected_.reset();
        changed = true;
      }
      else
        openRemove_ = true;
    }
    if (ImGui::MenuItem(character ? "Show in Character window" : "Show in Object window", nullptr, false, valid)){
      requests.show = true;
      requests.showKind = character ? ResourceID::Character : ResourceID::Object;
      requests.showName = character ? project.roomCharacters(room.name)[selected_->index].character : room.objects[selected_->index].object;
    }
    if (ImGui::MenuItem(tr("Set to back"), nullptr, false, valid)){
      selected_->index = moveToBack(project, room, *selected_);
      changed = true;
    }
    if (ImGui::MenuItem(tr("Set to front"), nullptr, false, valid)){
      selected_->index = moveToFront(project, room, *selected_);
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (openRemove_){
    ImGui::OpenPopup(tr("Confirmation"));
    openRemove_ = false;
  }
  if (ImGui::BeginPopupModal(tr("Confirmation"), nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    const bool valid = selectionValid(project, room);
    if (valid){
      const bool character = selected_->character;
      const std::string name = character ? project.roomCharacters(room.name)[selected_->index].name : room.objects[selected_->index].name;
      ImGui::Text("Delete the %s \"%s\" from your room?", character ? "Character" : "Object", toUtf8(name).c_str());
    }
    const ImVec2 size(6*ImGui::GetFontSize(), 0);
    if (ImGui::Button(tr("Yes"), size)){
      if (valid){
        if (selected_->character)
          removeCharacterInstance(project, room.name, selected_->index);
        else
          removeObjectInstance(project, room, selected_->index);
        changed = true;
      }
      selected_.reset();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(tr("No"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  if (ImGui::BeginPopup("walkmap")){
    if (ImGui::MenuItem(tr("Free"), nullptr, penFree_))
      penFree_ = true;
    if (ImGui::MenuItem(tr("Block"), nullptr, !penFree_))
      penFree_ = false;
    ImGui::Separator();
    const char* pens[] = {"Pen Size 1", "Pen Size 2", "Pen Size 3", "Script Paster", "Script Eraser"};
    for (int i = 0; i < 5; ++i){
      // the paster needs a copied script
      if (ImGui::MenuItem(pens[i], nullptr, (int)pen_ == i, i != 3 || scriptCopied_))
        pen_ = (WalkmapPen)i;
    }
    ImGui::Separator();
    const bool inside = inWalkmap(room, menuField_);
    const bool hasScript = inside && room.walkmap[menuField_.x][menuField_.y].hasScript;
    if (ImGui::MenuItem(tr("Copy Walkmap Script"), nullptr, false, hasScript)){
      const Script* scr = project.scripts[Script::Walkmap].find(Script::walkmapScriptName(menuField_.x, menuField_.y, room.name));
      copiedScript_ = scr ? scr->text : std::optional<std::string>();
      scriptCopied_ = true;
    }
    if (ImGui::MenuItem(tr("Paste Walkmap Script"), nullptr, false, inside && scriptCopied_)){
      pasteWalkmapScript(project, room, menuField_, copiedScript_);
      changed = true;
    }
    if (ImGui::MenuItem(tr("Edit Script"), nullptr, false, inside)){
      walkmapScript(project, room, menuField_);
      requests.openScript = true;
      requests.scriptType = Script::Walkmap;
      requests.script = Script::walkmapScriptName(menuField_.x, menuField_.y, room.name);
      changed = true;
    }
    ImGui::EndPopup();
  }
  if (ImGui::BeginPopup("inventory")){
    if (ImGui::MenuItem(tr("Remove Field"))){
      room.hasInventory = false;
      changed = true;
    }
    ImGui::EndPopup();
  }
  return changed;
}

bool RoomEditor::panel(Project& project, Room& room, Requests& requests){
  bool changed = false;
  const ProjectSettings& s = project.settings;
  ImGui::PushItemWidth(-7*ImGui::GetFontSize());
  if (ImGui::CollapsingHeader(tr("Room"), ImGuiTreeNodeFlags_DefaultOpen)){
    changed |= inputInt("Width", room.size.x, windowWidth(s), s.resolution.x*3);
    changed |= inputInt("Height", room.size.y, s.resolution.y, s.resolution.y*2);
    char zoom[16];
    snprintf(zoom, sizeof(zoom), "%d", room.zoom);
    if (ImGui::BeginCombo(tr("Zoom factor"), zoom)){
      for (int i = 1; i <= 9; ++i){
        snprintf(zoom, sizeof(zoom), "%d", i);
        if (ImGui::Selectable(zoom, room.zoom == i) && room.zoom != i){
          room.zoom = i;
          changed = true;
        }
      }
      ImGui::EndCombo();
    }
    changed |= checkbox("Double walkmap", room.doubleWalkmap);
    float light[3] = {room.lighting[0]/255.0f, room.lighting[1]/255.0f, room.lighting[2]/255.0f};
    if (ImGui::ColorEdit3(tr("Room light"), light)){
      for (int i = 0; i < 3; ++i)
        room.lighting[i] = (uint8_t)std::clamp((int)std::lround(light[i]*255), 0, 255);
      changed = true;
    }
    // the scroller: the room (grey) in the largest room there can be, and the window (white)
    const float width = ImGui::GetContentRegionAvail().x;
    const float height = width*2.0f/3.0f;
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("scroller", ImVec2(width, height));
    const float sx = width/(3.0f*s.resolution.x), sy = height/(2.0f*s.resolution.y);
    ImDrawList* list = ImGui::GetWindowDrawList();
    list->AddRectFilled(p, ImVec2(p.x+width, p.y+height), IM_COL32(240, 240, 240, 255));
    list->AddRect(p, ImVec2(p.x+width, p.y+height), kBlack);
    ImVec2 roomEnd(p.x + room.size.x*sx, p.y + room.size.y*sy);
    list->AddRectFilled(p, roomEnd, kLightGray);
    list->AddRect(p, roomEnd, kBlack);
    ImVec2 viewStart(p.x + room.scrollOffset.x*sx, p.y + room.scrollOffset.y*sy);
    ImVec2 viewEnd(viewStart.x + windowWidth(s)*sx, viewStart.y + s.resolution.y*sy);
    list->AddRectFilled(viewStart, viewEnd, kWhite);
    list->AddRect(viewStart, viewEnd, kBlack);
    if (ImGui::IsItemActivated()){
      scrolling_ = true;
      scrollStart_ = ImGui::GetIO().MousePos;
      scrollOffsetStart_ = room.scrollOffset;
    }
    if (scrolling_ && ImGui::IsItemActive()){
      ImVec2 m = ImGui::GetIO().MousePos;
      Vec2i offset{scrollOffsetStart_.x + (int)((m.x-scrollStart_.x)/sx), scrollOffsetStart_.y + (int)((m.y-scrollStart_.y)/sy)};
      offset.x = std::clamp(offset.x, 0, std::max(room.size.x - windowWidth(s), 0));
      offset.y = std::clamp(offset.y, 0, std::max(room.size.y - s.resolution.y, 0));
      if (!(offset == room.scrollOffset)){
        room.scrollOffset = offset;
        changed = true;
      }
    }
    else
      scrolling_ = false;
    ImGui::Text("Scroll offset: %d, %d", room.scrollOffset.x, room.scrollOffset.y);
    if (ImGui::Button(tr("Edit room script"))){
      requests.openScript = true;
      requests.scriptType = Script::Room;
      requests.script = room.name;
    }
  }

  if (mode_ == RoomMode::Objects || mode_ == RoomMode::Walkmap || mode_ == RoomMode::Deepmap){
    const bool valid = selectionValid(project, room);
    const bool obj = valid && !selected_->character;
    const bool chr = valid && selected_->character;
    if (ImGui::CollapsingHeader(tr("Object"), ImGuiTreeNodeFlags_DefaultOpen)){
      ImGui::BeginDisabled(!obj);
      if (obj){
        ObjectInstance& inst = room.objects[selected_->index];
        ImGui::Text("Instancename: %s", toUtf8(inst.name).c_str());
        changed |= checkbox("Locked##object", inst.locked);
        char state[16];
        snprintf(state, sizeof(state), "%d", inst.state);
        if (ImGui::BeginCombo(tr("State"), state)){
          for (int i = 0; i <= 10; ++i){
            snprintf(state, sizeof(state), "%d", i);
            if (ImGui::Selectable(state, inst.state == i) && inst.state != i){
              inst.state = i;
              changed = true;
            }
          }
          ImGui::EndCombo();
        }
        ImGui::TextUnformatted(tr("Drawstyle"));
        if (radioButtons(inst.layer, {"Back", "Middle", "Front"}, true)){
          selected_->depth = objectDepth(inst);
          changed = true;
        }
        if (ImGui::Button(tr("Edit script##object"))){
          requests.openScript = true;
          requests.scriptType = Script::Object;
          requests.script = Script::objectScriptName(inst.name, room.name);
        }
      }
      else
        ImGui::TextDisabled("Instancename: none");
      ImGui::EndDisabled();
    }
    if (ImGui::CollapsingHeader(tr("Character"), ImGuiTreeNodeFlags_DefaultOpen)){
      ImGui::BeginDisabled(!chr);
      if (chr){
        CharacterInstance& inst = (*project.characterInstances.find(room.name))[selected_->index];
        ImGui::Text("Instancename: %s", toUtf8(inst.name).c_str());
        changed |= checkbox("Locked##character", inst.locked);
        changed |= checkbox("Unmovable", inst.unmovable);
        for (int i = 0; i < 4; ++i){
          if (i > 0)
            ImGui::SameLine();
          if (ImGui::RadioButton(kLookDirs[i], inst.lookDir == i+1) && inst.lookDir != i+1){
            setLookDir(project, inst, i+1);
            changed = true;
          }
        }
        if (ImGui::Button(tr("Edit script##character"))){
          requests.openScript = true;
          requests.scriptType = Script::Character;
          requests.script = inst.name;
        }
      }
      else
        ImGui::TextDisabled("Instancename: none");
      ImGui::EndDisabled();
    }
    if (mode_ == RoomMode::Walkmap && ImGui::CollapsingHeader(tr("Walkmap"), ImGuiTreeNodeFlags_DefaultOpen)){
      int free = penFree_ ? 0 : 1;
      if (radioButtons(free, {"Free", "Block"}, true))
        penFree_ = free == 0;
      int pen = (int)pen_;
      if (radioButtons(pen, {"Pen Size 1", "Pen Size 2", "Pen Size 3", "Script Paster", "Script Eraser"})){
        if (pen != 3 || scriptCopied_)
          pen_ = (WalkmapPen)pen;
      }
      ImGui::TextDisabled("Right click for the scripts' menu, double click edits a field's script.");
    }
  }
  else if (mode_ == RoomMode::Inventory){
    if (ImGui::CollapsingHeader(tr("Inventory field"), ImGuiTreeNodeFlags_DefaultOpen))
      changed |= inputInt("Spacing", room.invSpacing, 2, 20);
  }
  else if (mode_ == RoomMode::SpecialFx && ImGui::BeginTabBar("shapes")){
    for (size_t i = 0; i < room.fxShapes.size(); ++i){
      char name[24];
      snprintf(name, sizeof(name), "Shape %d", (int)i+1);
      if (!ImGui::BeginTabItem(name))
        continue;
      FxShape& shape = room.fxShapes[i];
      ImGui::PushID((int)i);
      changed |= checkbox("Activate", shape.active);
      ImGui::BeginDisabled(!shape.active);
      const char* effects[] = {"Floor mirror", "Wall mirror", "Particle barrier"};
      int effect = std::clamp(shape.effect, 0, 2);
      if (ImGui::Combo(tr("Use effect"), &effect, effects, 3) && effect != shape.effect){
        shape.effect = effect;
        changed = true;
      }
      ImGui::BeginDisabled(shape.effect == FxShape::ParticleBarrier);
      int strength = shape.strength;
      if (ImGui::SliderInt("Strength", &strength, 0, 255) && strength != shape.strength){
        shape.strength = strength;
        changed = true;
      }
      ImGui::EndDisabled();
      const bool wall = shape.effect == FxShape::WallMirror;
      ImGui::BeginDisabled(!wall);
      changed |= checkbox("Depending on room position", shape.dependingOnRoomPosition);
      // the mirror image's offset: the light figure beside the dark one, dragged
      ImGui::TextUnformatted(tr("Position:"));
      const ImVec2 pad(100, 100);
      const ImVec2 p = ImGui::GetCursorScreenPos();
      ImGui::InvisibleButton("position", pad);
      ImDrawList* list = ImGui::GetWindowDrawList();
      list->AddRectFilled(p, ImVec2(p.x+pad.x, p.y+pad.y), IM_COL32(230, 230, 230, 255));
      list->AddRect(p, ImVec2(p.x+pad.x, p.y+pad.y), kBlack);
      if (wall){
        Vec2i off{shape.mirrorOffset.x/2, shape.mirrorOffset.y/2};
        if (shape.dependingOnRoomPosition)
          off.x = 0;
        ImVec2 light(p.x+26+off.x, p.y+27+off.y);
        list->AddRectFilled(light, ImVec2(light.x+40, light.y+46), IM_COL32(170, 170, 170, 255), 6);
        ImVec2 dark(p.x+32, p.y+27);
        list->AddRectFilled(dark, ImVec2(dark.x+40, dark.y+46), IM_COL32(60, 60, 60, 255), 6);
      }
      if (ImGui::IsItemActivated()){
        padShape_ = (int)i;
        padStart_ = ImGui::GetIO().MousePos;
        padOffsetStart_ = shape.mirrorOffset;
      }
      if (padShape_ == (int)i && ImGui::IsItemActive()){
        ImVec2 m = ImGui::GetIO().MousePos;
        Vec2i off{std::clamp(padOffsetStart_.x + (int)(m.x-padStart_.x), -100, 100), std::clamp(padOffsetStart_.y + (int)(m.y-padStart_.y), -100, 100)};
        if (shape.dependingOnRoomPosition)
          off.x = shape.mirrorOffset.x;
        if (!(off == shape.mirrorOffset)){
          shape.mirrorOffset = off;
          changed = true;
        }
      }
      else if (padShape_ == (int)i)
        padShape_ = -1;
      ImGui::EndDisabled();
      ImGui::EndDisabled();
      ImGui::PopID();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::PopItemWidth();
  return changed;
}

}
