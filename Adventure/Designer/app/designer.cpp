#include "designer.h"

#include <cstdio>
#include <filesystem>

#include <imgui_internal.h>

#include <model/projectfile.h>

namespace sd{

namespace{

const char* const kGamePool = "Game pool";
const char* const kMediaPool = "Media pool";
const char* const kSettings = "Project settings";

//! a project text (Windows-1252) as ImGui text
void text(const std::string& str){
  std::string utf8 = toUtf8(str);
  ImGui::TextUnformatted(utf8.c_str());
}

//! a label and a value in a two-column table row
void row(const char* label, const std::string& value){
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::TextUnformatted(label);
  ImGui::TableNextColumn();
  text(value);
}

void row(const char* label, int value) {row(label, std::to_string(value));}
void row(const char* label, bool value) {row(label, std::string(value ? "yes" : "no"));}
void row(const char* label, const Vec2i& v) {row(label, std::to_string(v.x) + ", " + std::to_string(v.y));}

std::string colorText(uint32_t color){
  //the files' colours are Windows' COLORREF: red in the low byte
  char buf[16];
  snprintf(buf, sizeof(buf), "#%02X%02X%02X", color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
  return buf;
}

bool beginTable(const char* id){
  return ImGui::BeginTable(id, 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV);
}

const char* kindName(ResourceID kind){
  switch (kind){
    case ResourceID::Script: return "Cutscene";
    case ResourceID::Item: return "Item";
    case ResourceID::Music: return "Music";
    case ResourceID::Object: return "Object";
    case ResourceID::Image: return "Image";
    case ResourceID::Room: return "Room";
    case ResourceID::Sound: return "Sound";
    case ResourceID::Folder: return "Folder";
    case ResourceID::Character: return "Character";
    case ResourceID::Video: return "Video";
  }
  return "Resource";
}

std::string parentFolder(const std::string& path){
  std::filesystem::path p(std::u8string(path.begin(), path.end()));
  std::u8string str = p.parent_path().u8string();
  return std::string(str.begin(), str.end());
}

}

std::string Designer::View::title() const{
  //the name shown, the kind and name as the window's id
  return toUtf8(name) + " (" + kindName(kind) + ")###" + kindName(kind) + "/" + toUtf8(toLower(name));
}

Designer::Designer(){
}

bool Designer::openProject(const std::string& path){
  Project project;
  std::string error;
  if (!readProject(path, project, &error)){
    error_ = error;
    return false;
  }
  project_ = std::move(project);
  path_ = path;
  views_.clear();
  return true;
}

bool Designer::save(const std::string& path){
  std::string error;
  if (!writeProject(path, project_, &error)){
    error_ = error;
    return false;
  }
  path_ = path;
  return true;
}

void Designer::gui(){
  menu();
  ImGuiID dockspace = ImGui::DockSpaceOverViewport();
  if (!layoutDone_){
    layout(dockspace);
    layoutDone_ = true;
  }
  if (showGamePool_)
    pool(kGamePool, project_.gamePool, &showGamePool_);
  if (showMediaPool_)
    pool(kMediaPool, project_.mediaPool, &showMediaPool_);
  if (showSettings_)
    settingsWindow();
  for (View& v : views_)
    view(v);
  //closed windows go after the frame
  for (size_t i = views_.size(); i-- > 0;){
    if (!views_[i].open)
      views_.erase(views_.begin()+i);
  }
  if (fileDialog_.draw()){
    if (saving_)
      save(fileDialog_.path());
    else
      openProject(fileDialog_.path());
  }
  errorPopup();
}

void Designer::menu(){
  if (!ImGui::BeginMainMenuBar())
    return;
  if (ImGui::BeginMenu("File")){
    if (ImGui::MenuItem("New")){
      project_ = Project();
      path_.clear();
      views_.clear();
    }
    if (ImGui::MenuItem("Open...")){
      saving_ = false;
      fileDialog_.open(FileDialog::Mode::Open, "Open a project", parentFolder(path_), {".adv", ".dat"});
    }
    if (ImGui::MenuItem("Save", nullptr, false, !path_.empty()))
      save(path_);
    if (ImGui::MenuItem("Save as...")){
      saving_ = true;
      fileDialog_.open(FileDialog::Mode::Save, "Save the project as", parentFolder(path_), {".adv"});
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit"))
      quit_ = true;
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Project")){
    ImGui::MenuItem("Settings", nullptr, &showSettings_);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("View")){
    ImGui::MenuItem(kGamePool, nullptr, &showGamePool_);
    ImGui::MenuItem(kMediaPool, nullptr, &showMediaPool_);
    ImGui::EndMenu();
  }
  //the project's name on the right
  std::string name = toUtf8(project_.settings.projectName);
  float width = ImGui::CalcTextSize(name.c_str()).x;
  ImGui::SameLine(ImGui::GetWindowWidth() - width - ImGui::GetStyle().ItemSpacing.x*2);
  ImGui::TextDisabled("%s", name.c_str());
  ImGui::EndMainMenuBar();
}

void Designer::layout(ImGuiID dockspace){
  //the pools on the left, game pool above media pool; the resources on the right
  ImGui::DockBuilderRemoveNode(dockspace);
  ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetMainViewport()->WorkSize);
  ImGuiID left, right;
  ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.25f, &left, &right);
  ImGuiID top, bottom;
  ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.6f, &top, &bottom);
  ImGui::DockBuilderDockWindow(kGamePool, top);
  ImGui::DockBuilderDockWindow(kMediaPool, bottom);
  ImGui::DockBuilderDockWindow(kSettings, right);
  ImGui::DockBuilderFinish(dockspace);
  mainDock_ = right;
}

void Designer::pool(const char* title, std::vector<PoolNode>& nodes, bool* open){
  if (ImGui::Begin(title, open)){
    if (nodes.empty())
      ImGui::TextDisabled("(empty)");
    poolNodes(nodes);
  }
  ImGui::End();
}

void Designer::poolNodes(std::vector<PoolNode>& nodes){
  for (PoolNode& node : nodes){
    std::string label = toUtf8(node.label);
    ImGui::PushID(&node);
    if (node.id == (int)ResourceID::Folder){
      if (ImGui::TreeNodeEx("##folder", ImGuiTreeNodeFlags_SpanAvailWidth, "%s/", label.c_str())){
        poolNodes(node.children);
        ImGui::TreePop();
      }
    }
    else{
      ImGui::TreeNodeEx("##leaf", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        openView((ResourceID)node.id, node.label);
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", kindName((ResourceID)node.id));
    }
    ImGui::PopID();
  }
}

void Designer::openView(ResourceID kind, const std::string& name){
  for (View& v : views_){
    if (v.kind == kind && toLower(v.name) == toLower(name)){
      ImGui::SetWindowFocus(v.title().c_str());
      return;
    }
  }
  View v;
  v.kind = kind;
  v.name = name;
  views_.push_back(v);
}

void Designer::view(View& v){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(40*ImGui::GetFontSize(), 30*ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
  if (ImGui::Begin(v.title().c_str(), &v.open)){
    switch (v.kind){
      case ResourceID::Room:
        if (const Room* room = project_.rooms.find(v.name))
          showRoom(*room);
        else
          ImGui::TextDisabled("There is no room of this name.");
        break;
      case ResourceID::Object:
        if (const Object* obj = project_.objects.find(v.name))
          showObject(*obj);
        else
          ImGui::TextDisabled("There is no object of this name.");
        break;
      case ResourceID::Item:
        if (const Item* item = project_.items.find(v.name))
          showItem(*item);
        else
          ImGui::TextDisabled("There is no item of this name.");
        break;
      case ResourceID::Character:
        if (const Character* chr = project_.characters.find(v.name))
          showCharacter(*chr);
        else
          ImGui::TextDisabled("There is no character of this name.");
        break;
      case ResourceID::Script:
        showScript(project_.scripts[Script::Cutscene].find(v.name));
        break;
      default:{
        NamedList<std::string>* files = nullptr;
        if (v.kind == ResourceID::Image)
          files = &project_.images;
        else if (v.kind == ResourceID::Sound)
          files = &project_.sounds;
        else if (v.kind == ResourceID::Music)
          files = &project_.music;
        else if (v.kind == ResourceID::Video)
          files = &project_.videos;
        const std::string* file = files ? files->find(v.name) : nullptr;
        if (beginTable("file")){
          row("File", file ? *file : std::string("(none)"));
          ImGui::EndTable();
        }
        break;
      }
    }
  }
  ImGui::End();
}

void Designer::showRoom(const Room& room){
  if (ImGui::CollapsingHeader("Room", ImGuiTreeNodeFlags_DefaultOpen) && beginTable("room")){
    row("Size", room.size);
    row("Scroll offset", room.scrollOffset);
    row("Background", room.background);
    row("Parallax background", room.parallaxBackground);
    row("Depth map", room.depthmap);
    row("Zoom", room.zoom);
    row("Double walkmap", room.doubleWalkmap);
    row("Lighting", std::to_string(room.lighting[0]) + ", " + std::to_string(room.lighting[1]) + ", " + std::to_string(room.lighting[2]));
    row("Inventory", room.hasInventory);
    if (room.hasInventory){
      row("Inventory position", room.invPos);
      row("Inventory size", room.invSize);
      char scale[64];
      snprintf(scale, sizeof(scale), "%g, %g", room.invScale.x, room.invScale.y);
      row("Inventory scale", std::string(scale));
      row("Inventory spacing", room.invSpacing);
    }
    int free = 0, scripts = 0;
    for (const auto& column : room.walkmap){
      for (const WalkMapEntry& field : column){
        free += field.isFree ? 1 : 0;
        scripts += field.hasScript ? 1 : 0;
      }
    }
    row("Walkable fields", free);
    row("Fields with scripts", scripts);
    ImGui::EndTable();
  }
  if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen)){
    if (ImGui::BeginTable("objects", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)){
      for (const char* header : {"Name", "Object", "Position", "State", "Layer", "Depth", "Locked"})
        ImGui::TableSetupColumn(header);
      ImGui::TableHeadersRow();
      for (const ObjectInstance& obj : room.objects){
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); text(obj.name);
        ImGui::TableNextColumn(); text(obj.object);
        ImGui::TableNextColumn(); ImGui::Text("%d, %d", obj.position.x, obj.position.y);
        ImGui::TableNextColumn(); ImGui::Text("%d", obj.state);
        ImGui::TableNextColumn(); ImGui::Text("%d", obj.layer);
        ImGui::TableNextColumn(); ImGui::Text("%d", obj.depth);
        ImGui::TableNextColumn(); ImGui::TextUnformatted(obj.locked ? "yes" : "no");
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::CollapsingHeader("Characters", ImGuiTreeNodeFlags_DefaultOpen)){
    const std::vector<CharacterInstance>& chars = project_.roomCharacters(room.name);
    if (ImGui::BeginTable("characters", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)){
      for (const char* header : {"Name", "Character", "Position", "Looks", "Locked"})
        ImGui::TableSetupColumn(header);
      ImGui::TableHeadersRow();
      for (const CharacterInstance& chr : chars){
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); text(chr.name);
        ImGui::TableNextColumn(); text(chr.character);
        ImGui::TableNextColumn(); ImGui::Text("%d, %d", chr.position.x, chr.position.y);
        ImGui::TableNextColumn(); ImGui::Text("%d", chr.lookDir);
        ImGui::TableNextColumn(); ImGui::TextUnformatted(chr.locked ? "yes" : "no");
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::CollapsingHeader("FX shapes")){
    for (size_t i = 0; i < room.fxShapes.size(); ++i){
      const FxShape& shape = room.fxShapes[i];
      ImGui::PushID((int)i);
      if (beginTable("shape")){
        row("Shape", (int)i+1);
        row("Active", shape.active);
        row("Effect", shape.effect);
        row("Depth", shape.depth);
        row("Strength", shape.strength);
        ImGui::EndTable();
      }
      ImGui::PopID();
    }
  }
  if (ImGui::CollapsingHeader("Script"))
    showScript(project_.scripts[Script::Room].find(room.name));
}

void Designer::showFrames(const std::vector<ExtendedFrame>& frames){
  for (size_t f = 0; f < frames.size(); ++f){
    std::string parts;
    for (size_t p = 0; p < frames[f].names.size(); ++p){
      if (p > 0)
        parts += " + ";
      parts += frames[f].names[p];
    }
    ImGui::BulletText("%zu: %s", f+1, toUtf8(parts).c_str());
  }
}

void Designer::showObject(const Object& obj){
  if (beginTable("object")){
    row("Size", obj.size);
    row("Lighten", obj.lighten);
    ImGui::EndTable();
  }
  for (size_t s = 0; s < obj.states.size(); ++s){
    const ObjectState& state = obj.states[s];
    if (state.frames.empty())
      continue;
    char label[64];
    snprintf(label, sizeof(label), "State %zu (%zu frames, every %d)", s+1, state.frames.size(), state.fpsDivider);
    if (ImGui::TreeNode(label)){
      showFrames(state.frames);
      ImGui::TreePop();
    }
  }
}

void Designer::showItem(const Item& item){
  for (size_t s = 0; s < item.states.size(); ++s){
    const ItemState& state = item.states[s];
    if (state.frames.empty())
      continue;
    char label[64];
    snprintf(label, sizeof(label), "State %zu (%zu frames, every %d)", s+1, state.frames.size(), state.fpsDivider);
    if (ImGui::TreeNode(label)){
      for (size_t f = 0; f < state.frames.size(); ++f)
        ImGui::BulletText("%zu: %s", f+1, toUtf8(state.frames[f]).c_str());
      ImGui::TreePop();
    }
  }
  if (ImGui::CollapsingHeader("Script"))
    showScript(project_.scripts[Script::Item].find(item.name));
}

void Designer::showCharacter(const Character& chr){
  if (beginTable("character")){
    row("Text colour", colorText(chr.textColor));
    row("Walk speed", chr.walkSpeed);
    row("Zoom", chr.zoom);
    row("Not zoomed", chr.noZoom);
    row("Real left animations", chr.realLeftAnimations);
    row("Memory resistant", chr.memoryResistant);
    row("Ghost", chr.ghost);
    row("Walk sound", chr.walkSound);
    row("Font", chr.font);
    ImGui::EndTable();
  }
  for (size_t s = 0; s < chr.states.size(); ++s){
    const CharacterState& state = chr.states[s];
    if (state.frames.empty())
      continue;
    char label[96];
    snprintf(label, sizeof(label), "State %zu (%zu frames, size %d x %d)", s+1, state.frames.size(), state.size.x, state.size.y);
    if (ImGui::TreeNode(label)){
      showFrames(state.frames);
      ImGui::TreePop();
    }
  }
  if (ImGui::CollapsingHeader("Script"))
    showScript(project_.scripts[Script::Character].find(chr.name));
}

void Designer::showScript(const Script* scr){
  if (!scr || !scr->text){
    ImGui::TextDisabled("(no script)");
    return;
  }
  std::string utf8 = toUtf8(*scr->text);
  ImGui::InputTextMultiline("##script", utf8.data(), utf8.size()+1, ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_ReadOnly);
}

void Designer::settingsWindow(){
  if (ImGui::Begin(kSettings, &showSettings_)){
    const ProjectSettings& s = project_.settings;
    if (beginTable("settings")){
      row("Project", s.projectName);
      row("Resolution", s.resolution);
      row("16:9", s.is16to9);
      row("Start script", s.startScript);
      row("Main script", s.mainScript);
      row("Anywhere room", s.anywhereRoom);
      row("Screen change", (int)s.screenChange);
      row("Task bar room", s.taskRoom);
      row("Task bar height", s.taskHeight);
      row("Menu room", s.customMenu ? s.customMenuRoom : std::string("(standard)"));
      row("Text colour", colorText(s.textColor));
      row("Walk text", s.walkText);
      row("Link text", s.linkText);
      row("Give link", s.giveLink);
      row("Scripting language", std::string(s.scriptingLanguage == ScriptLang::Lua ? "Lua" : "PCDK"));
      row("Protected data files", s.protectGameFile);
      ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("Commands")){
      for (const auto& cmd : s.commands)
        ImGui::BulletText("%s: %s", toUtf8(cmd.first).c_str(), toUtf8(cmd.second).c_str());
    }
    if (ImGui::CollapsingHeader("Booleans")){
      for (const auto& b : s.booleans)
        ImGui::BulletText("%s = %s", toUtf8(b.first).c_str(), b.second ? "true" : "false");
    }
    if (ImGui::CollapsingHeader("Fonts")){
      for (const FontInfo& font : s.fonts)
        ImGui::BulletText("%s, %d%s%s", toUtf8(font.name).c_str(), font.size, font.bold ? ", bold" : "", font.italic ? ", italic" : "");
    }
  }
  ImGui::End();
}

void Designer::errorPopup(){
  if (!error_.empty() && !ImGui::IsPopupOpen("Error"))
    ImGui::OpenPopup("Error");
  if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    ImGui::TextUnformatted(error_.c_str());
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0))){
      error_.clear();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

}
