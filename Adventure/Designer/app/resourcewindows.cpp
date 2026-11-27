// the views of items, objects, characters and images, and the mouse icons
// window: their states and frames in the state and frame editor

#include "designer.h"

#include <cfloat>
#include <cstdio>
#include <filesystem>

#include <model/images.h>
#include <model/stateframes.h>

#include "widgets.h"

namespace sd{

namespace{

//! ImGui's colour (ABGR) from the picture's background
ImVec4 toVec4(ImU32 color){
  return ImGui::ColorConvertU32ToFloat4(color);
}

bool beginPropertyTable(const char* id){
  if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp))
    return false;
  ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
  ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
  return true;
}

//! a row with its label, the value's widget follows
void propertyRow(const char* label){
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(label);
  ImGui::TableNextColumn();
}

}

std::string Designer::projectFolder() const{
  if (path_.empty())
    return std::string();
  std::filesystem::path p(std::u8string(path_.begin(), path_.end()));
  std::u8string str = p.parent_path().u8string();
  return std::string(str.begin(), str.end());
}

StateFrameEditor& Designer::frameEditor(const char* kind, const std::string& name){
  return frameEditors_[std::string(kind) + "/" + toLower(name)];
}

void Designer::showItem(Item& item){
  ItemFrames data(item);
  StateFrameStyle style;
  style.scaleToBox = true;
  style.sizeFields = false;
  if (frameEditor("item", item.name).draw(data, style, images_, project_, projectFolder()))
    changed();
  ImGui::Separator();
  if (ImGui::CollapsingHeader("Script"))
    showScript(Script::Item, item.name);
}

void Designer::showObject(Object& obj){
  ObjectFrames data(obj);
  StateFrameStyle style;
  style.adaptFirstDrop = true;
  if (frameEditor("object", obj.name).draw(data, style, images_, project_, projectFolder()))
    changed();
  ImGui::Separator();
  if (checkbox("Lighten object", obj.lighten))
    changed();
}

void Designer::showCharacter(Character& chr){
  CharacterFrames data(chr);
  StateFrameEditor& editor = frameEditor("character", chr.name);
  StateFrameStyle style;
  for (int s = 0; s < data.states(); ++s)
    style.labels.push_back(characterStateName(chr, s));
  style.hotspot = true;
  style.background = charBackground_;

  //the C# dialog's Customize menu
  if (ImGui::Button("Customize"))
    ImGui::OpenPopup("customize");
  if (ImGui::BeginPopup("customize")){
    int state = editor.state();
    if (ImGui::MenuItem("Copy action")){
      copiedState_ = copyState(data, state);
      stateCopied_ = true;
    }
    if (ImGui::MenuItem("Paste action", nullptr, false, stateCopied_)){
      pasteState(data, state, copiedState_);
      changed();
    }
    if (ImGui::MenuItem("Delete action")){
      clearState(data, state);
      changed();
    }
    if (ImGui::MenuItem("Rename action", nullptr, false, state >= 16)){
      renameState_ = state;
      snprintf(stateNameBuf_, sizeof(stateNameBuf_), "%s", toUtf8(chr.extraStateNames[state-16]).c_str());
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Equalize all sizes and footprints")){
      Vec2i size = data.size(state), spot = data.hotspot(state);
      for (int s = 0; s < data.states(); ++s){
        data.setSize(s, size);
        data.setHotspot(s, spot);
      }
      changed();
    }
    if (ImGui::MenuItem("Equalize all actions (like selected)")){
      CopiedState copy = copyState(data, state);
      for (int s = 0; s < data.states(); ++s)
        pasteState(data, s, copy);
      changed();
    }
    ImGui::Separator();
    ImVec4 bg = toVec4(charBackground_);
    if (ImGui::ColorEdit3("Background colour", &bg.x, ImGuiColorEditFlags_NoInputs))
      charBackground_ = ImGui::ColorConvertFloat4ToU32(bg);
    ImGui::EndPopup();
  }
  if (renameState_ >= 0){
    ImGui::SameLine();
    ImGui::SetNextItemWidth(12*ImGui::GetFontSize());
    if (ImGui::IsWindowAppearing() || !ImGui::IsAnyItemActive())
      ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("New name (Enter)", stateNameBuf_, sizeof(stateNameBuf_), ImGuiInputTextFlags_EnterReturnsTrue)){
      if (renameState_-16 < (int)chr.extraStateNames.size()){
        chr.extraStateNames[renameState_-16] = fromUtf8(stateNameBuf_);
        changed();
      }
      renameState_ = -1;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
      renameState_ = -1;
  }

  if (editor.draw(data, style, images_, project_, projectFolder()))
    changed();
  ImGui::Separator();

  if (beginPropertyTable("character")){
    propertyRow("Text colour");
    if (colorEdit("##textcolor", chr.textColor))
      changed();
    propertyRow("Font");
    {
      std::vector<std::string> fonts{"Default"};
      for (const FontInfo& fi : project_.settings.fonts)
        fonts.push_back(toUtf8(fi.name) + " (" + std::to_string(fi.size) + ")");
      int font = chr.font;
      ImGui::SetNextItemWidth(-FLT_MIN);
      if (ImGui::BeginCombo("##font", font >= 0 && font < (int)fonts.size() ? fonts[font].c_str() : "")){
        for (int i = 0; i < (int)fonts.size(); ++i){
          if (ImGui::Selectable(fonts[i].c_str(), i == font)){
            chr.font = i;
            changed();
          }
        }
        ImGui::EndCombo();
      }
    }
    propertyRow("Walk speed");
    {
      ImGui::SetNextItemWidth(-FLT_MIN);
      std::string speed = chr.walkSpeed >= 1 && chr.walkSpeed <= 9 ? std::to_string(chr.walkSpeed) : "";
      if (ImGui::BeginCombo("##walkspeed", speed.c_str())){
        for (int i = 1; i < 10; ++i){
          if (ImGui::Selectable(std::to_string(i).c_str(), i == chr.walkSpeed)){
            chr.walkSpeed = i;
            changed();
          }
        }
        ImGui::EndCombo();
      }
    }
    propertyRow("Walking sound");
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (inputText("##walksound", chr.walkSound))
      changed();
    propertyRow("Start zoom (%)");
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (inputInt("##zoom", chr.zoom, 0, 500))
      changed();
    ImGui::EndTable();
  }
  if (checkbox("Do NOT zoom", chr.noZoom))
    changed();
  ImGui::SameLine();
  if (checkbox("Real left animations", chr.realLeftAnimations))
    changed();
  if (checkbox("Memory resistant", chr.memoryResistant))
    changed();
  ImGui::SameLine();
  if (checkbox("Ghost mode", chr.ghost))
    changed();
  ImGui::Separator();
  if (ImGui::CollapsingHeader("Script"))
    showScript(Script::Character, chr.name);
}

void Designer::showImage(const std::string& name){
  const ImageCache::Image& img = images_.get(project_, projectFolder(), name);
  if (!img.texture){
    ImGui::TextDisabled("The image can't be found: not at its path, next to the project, or in its gfx.dat.");
    return;
  }
  ImGui::Text("%d x %d pixels", img.width, img.height);
  ImGui::Image(img.id(), ImVec2((float)img.width, (float)img.height));
}

void Designer::mouseIconsWindow(){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  if (ImGui::Begin(kMouseIcons, &showMouseIcons_)){
    CursorFrames data(project_.cursor);
    StateFrameStyle style;
    style.labels = {"S", "1", "2", "3", "4", "5", "6", "7", "8", "L"};
    style.scaleToBox = true;
    style.sizeFields = false;
    style.hotspot = true;
    style.hotspotScale = 3.0f;
    style.scripts = false;
    style.animation = false;
    style.imageNames = false;
    StateFrameEditor& editor = frameEditor("cursor", "");
    if (editor.draw(data, style, images_, project_, projectFolder()))
      changed();
    //the icon at its size beside the command
    ImGui::Separator();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(32, 32));
    editor.drawFrame(ImGui::GetWindowDrawList(), data, style, images_, project_, projectFolder(), pos, ImVec2(32, 32));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::TextUnformatted("32x32");
    int state = editor.state();
    CursorState& cs = project_.cursor[state];
    std::vector<std::string> commands{"walkto (standard)", "loading icon"};
    for (const auto& command : project_.settings.commands)
      commands.push_back(toUtf8(command.first));
    //the first and the last state are always walk to and the loading icon
    ImGui::BeginDisabled(state == 0 || state == 9);
    int index = cs.command-1;
    ImGui::SetNextItemWidth(16*ImGui::GetFontSize());
    if (ImGui::BeginCombo("Command", index >= 0 && index < (int)commands.size() ? commands[index].c_str() : "")){
      for (int i = 0; i < (int)commands.size(); ++i){
        if (ImGui::Selectable(commands[i].c_str(), i == index)){
          cs.command = i+1;
          changed();
        }
      }
      ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    ImGui::EndGroup();
    ImGui::SeparatorText("Right click...");
    if (radioButtons(project_.settings.rightClick, {"... changes the icons (classic)", "... selects icon 1 (intelligent)", "... does nothing"}))
      changed();
    ImGui::BeginDisabled(project_.settings.rightClick != 0);
    if (checkbox("Use mouse wheel", project_.settings.useMouseWheel))
      changed();
    ImGui::EndDisabled();
  }
  ImGui::End();
}

}
