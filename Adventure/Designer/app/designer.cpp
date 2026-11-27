#include "designer.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

#include <imgui_internal.h>

#include <model/languages.h>
#include <model/pcdkparser.h>
#include <model/projectfile.h>

#include "widgets.h"

namespace sd{

namespace{

const char* const kGamePool = "Game pool";
const char* const kMediaPool = "Media pool";

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
  scriptEditors_.clear();
  frameEditors_.clear();
  images_.clear();
  language_.clear();
  modified_ = false;
  return true;
}

bool Designer::save(const std::string& path){
  std::string error;
  if (!writeProject(path, project_, &error)){
    error_ = error;
    return false;
  }
  path_ = path;
  modified_ = false;
  return true;
}

void Designer::whenSaved(Pending action){
  pending_ = action;
  //asked in unsavedPopup
  if (!modified_)
    runPending();
}

void Designer::runPending(){
  Pending action = pending_;
  pending_ = Pending::None;
  switch (action){
    case Pending::New:
      project_ = Project();
      path_.clear();
      views_.clear();
      scriptEditors_.clear();
      language_.clear();
      modified_ = false;
      break;
    case Pending::Open:
      saving_ = false;
      fileDialog_.open(FileDialog::Mode::Open, "Open a project", parentFolder(path_), {".adv", ".dat"});
      break;
    case Pending::Quit:
      quit_ = true;
      break;
    case Pending::None:
      break;
  }
}

void Designer::unsavedPopup(){
  const char* const title = "Unsaved changes";
  //not while the save dialog of "Save" runs
  if (pending_ != Pending::None && modified_ && !fileDialog_.isOpen() && !ImGui::IsPopupOpen(title))
    ImGui::OpenPopup(title);
  if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return;
  ImGui::Text("Save the changes to %s?", toUtf8(project_.settings.projectName).c_str());
  const ImVec2 size(7*ImGui::GetFontSize(), 0);
  if (ImGui::Button("Save", size)){
    ImGui::CloseCurrentPopup();
    if (path_.empty()){
      saving_ = true;
      fileDialog_.open(FileDialog::Mode::Save, "Save the project as", parentFolder(path_), {".adv"});
    }
    else if (save(path_))
      runPending();
    else
      pending_ = Pending::None;
  }
  ImGui::SameLine();
  if (ImGui::Button("Don't save", size)){
    ImGui::CloseCurrentPopup();
    modified_ = false;
    runPending();
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel", size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
    ImGui::CloseCurrentPopup();
    pending_ = Pending::None;
  }
  ImGui::EndPopup();
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
  if (showCommands_)
    commandsWindow();
  if (showLanguages_)
    languagesWindow();
  if (showMouseIcons_)
    mouseIconsWindow();
  for (View& v : views_)
    view(v);
  //closed windows go after the frame
  for (size_t i = views_.size(); i-- > 0;){
    if (!views_[i].open)
      views_.erase(views_.begin()+i);
  }
  const bool dialogWasOpen = fileDialog_.isOpen();
  if (fileDialog_.draw()){
    if (saving_){
      if (save(fileDialog_.path()) && pending_ != Pending::None)
        runPending();
    }
    else
      openProject(fileDialog_.path());
  }
  else if (dialogWasOpen && !fileDialog_.isOpen() && saving_)
    pending_ = Pending::None; //the save was cancelled, and what waited for it
  unsavedPopup();
  namePopup();
  errorPopup();
}

void Designer::menu(){
  if (!ImGui::BeginMainMenuBar())
    return;
  if (ImGui::BeginMenu("File")){
    if (ImGui::MenuItem("New"))
      whenSaved(Pending::New);
    if (ImGui::MenuItem("Open..."))
      whenSaved(Pending::Open);
    if (ImGui::MenuItem("Save", nullptr, false, !path_.empty()))
      save(path_);
    if (ImGui::MenuItem("Save as...")){
      saving_ = true;
      fileDialog_.open(FileDialog::Mode::Save, "Save the project as", parentFolder(path_), {".adv"});
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit"))
      whenSaved(Pending::Quit);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Project")){
    ImGui::MenuItem("Settings", nullptr, &showSettings_);
    ImGui::MenuItem("Commands and bools", nullptr, &showCommands_);
    ImGui::MenuItem(kMouseIcons, nullptr, &showMouseIcons_);
    if (ImGui::MenuItem("Languages", nullptr, &showLanguages_) && showLanguages_){
      //the C# StoryDesigner gathers the texts when its language tools open
      updateLanguageList(project_);
      changed();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("View")){
    ImGui::MenuItem(kGamePool, nullptr, &showGamePool_);
    ImGui::MenuItem(kMediaPool, nullptr, &showMediaPool_);
    ImGui::EndMenu();
  }
  //the project's name on the right, with a star for unsaved changes
  std::string name = toUtf8(project_.settings.projectName) + (modified_ ? " *" : "");
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
  ImGui::DockBuilderDockWindow(kCommands, right);
  ImGui::DockBuilderDockWindow(kLanguages, right);
  ImGui::DockBuilderDockWindow(kMouseIcons, right);
  ImGui::DockBuilderFinish(dockspace);
  mainDock_ = right;
}

void Designer::pool(const char* title, std::vector<PoolNode>& nodes, bool* open){
  if (ImGui::Begin(title, open)){
    if (nodes.empty())
      ImGui::TextDisabled("(empty)");
    std::vector<size_t> path;
    poolNodes(nodes, &nodes == &project_.gamePool, path);
  }
  ImGui::End();
}

void Designer::poolNodes(std::vector<PoolNode>& nodes, bool game, std::vector<size_t>& path){
  //the actions change the pool: they wait until it is drawn
  std::vector<size_t> newScriptIn;
  bool newScript = false;
  for (size_t i = 0; i < nodes.size(); ++i){
    PoolNode& node = nodes[i];
    std::string label = toUtf8(node.label);
    ImGui::PushID((int)i);
    path.push_back(i);
    if (node.id == (int)ResourceID::Folder){
      bool open = ImGui::TreeNodeEx("##folder", ImGuiTreeNodeFlags_SpanAvailWidth, "%s/", label.c_str());
      if (game && ImGui::BeginPopupContextItem()){
        if (ImGui::MenuItem("New script")){
          newScript = true;
          newScriptIn = path;
        }
        ImGui::EndPopup();
      }
      if (open){
        poolNodes(node.children, game, path);
        ImGui::TreePop();
      }
    }
    else{
      ImGui::TreeNodeEx("##leaf", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        openView((ResourceID)node.id, node.label);
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", kindName((ResourceID)node.id));
      //images are dragged into the frames of items, objects, characters and the cursor
      if (node.id == (int)ResourceID::Image && ImGui::BeginDragDropSource()){
        ImGui::SetDragDropPayload(kImagePayload, node.label.data(), node.label.size());
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndDragDropSource();
      }
      if (node.id == (int)ResourceID::Script && ImGui::BeginPopupContextItem()){
        if (ImGui::MenuItem("Open"))
          openView(ResourceID::Script, node.label);
        if (ImGui::MenuItem("Rename...")){
          nameFor_ = NameFor::RenameScript;
          nameTarget_ = node.label;
          snprintf(nameBuf_, sizeof(nameBuf_), "%s", label.c_str());
        }
        if (ImGui::MenuItem("Delete")){
          deleteKind_ = ResourceID::Script;
          deleteName_ = node.label;
        }
        ImGui::EndPopup();
      }
    }
    path.pop_back();
    ImGui::PopID();
  }
  if (newScript)
    this->newScript(newScriptIn);
}

PoolNode* Designer::poolNode(std::vector<PoolNode>& nodes, const std::vector<size_t>& path){
  std::vector<PoolNode>* level = &nodes;
  PoolNode* node = nullptr;
  for (size_t idx : path){
    if (idx >= level->size())
      return nullptr;
    node = &(*level)[idx];
    level = &node->children;
  }
  return node;
}

namespace{

//! the pool's nodes sorted by their labels, as the C# tree view sorts them
void sortPool(std::vector<PoolNode>& nodes){
  std::stable_sort(nodes.begin(), nodes.end(), [](const PoolNode& a, const PoolNode& b){
    return toLower(a.label) < toLower(b.label);
  });
}

//! removes a resource's node from the pool; true if it was there
bool removeFromPool(std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name){
  for (size_t i = 0; i < nodes.size(); ++i){
    if (nodes[i].id == (int)kind && toLower(nodes[i].label) == toLower(name)){
      nodes.erase(nodes.begin()+i);
      return true;
    }
    if (removeFromPool(nodes[i].children, kind, name))
      return true;
  }
  return false;
}

//! the list of nodes that holds a resource
std::vector<PoolNode>* poolListOf(std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name){
  for (PoolNode& node : nodes){
    if (node.id == (int)kind && toLower(node.label) == toLower(name))
      return &nodes;
    if (std::vector<PoolNode>* found = poolListOf(node.children, kind, name))
      return found;
  }
  return nullptr;
}

PoolNode* findInPool(std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name){
  for (PoolNode& node : nodes){
    if (node.id == (int)kind && toLower(node.label) == toLower(name))
      return &node;
    if (PoolNode* found = findInPool(node.children, kind, name))
      return found;
  }
  return nullptr;
}

}

void Designer::newScript(const std::vector<size_t>& folder){
  //into the scripts' folder (the pool's second), as in the C# StoryDesigner
  PoolNode* parent = nullptr;
  if (!folder.empty() && folder[0] == 1)
    parent = poolNode(project_.gamePool, folder);
  else if (project_.gamePool.size() > 1)
    parent = &project_.gamePool[1];
  if (parent == nullptr || parent->id != (int)ResourceID::Folder)
    return;
  NamedList<Script>& scripts = project_.scripts[Script::Cutscene];
  int count = (int)scripts.size()+1;
  std::string name = "Script" + std::to_string(count);
  while (scripts.find(name) != nullptr)
    name = "Script" + std::to_string(++count);
  Script scr;
  scr.type = Script::Cutscene;
  scr.name = name;
  scr.text = std::string();
  scripts.add(name, scr);
  PoolNode node;
  node.id = (int)ResourceID::Script;
  node.label = name;
  parent->children.push_back(node);
  sortPool(parent->children);
  changed();
  openView(ResourceID::Script, name);
}

void Designer::deleteResource(ResourceID kind, const std::string& name){
  if (kind != ResourceID::Script)
    return;
  project_.scripts[Script::Cutscene].remove(name);
  removeFromPool(project_.gamePool, kind, name);
  scriptEditors_.erase(std::to_string(Script::Cutscene) + "/" + toLower(name));
  for (View& v : views_){
    if (v.kind == kind && toLower(v.name) == toLower(name))
      v.open = false;
  }
  changed();
}

bool Designer::renameScript(const std::string& from, const std::string& to){
  NamedList<Script>& scripts = project_.scripts[Script::Cutscene];
  if (to.empty()){
    error_ = "A script needs a name.";
    return false;
  }
  Script* scr = scripts.find(from);
  if (scr == nullptr)
    return false;
  if (toLower(from) != toLower(to) && scripts.find(to) != nullptr){
    error_ = "There is a script called " + toUtf8(to) + " already.";
    return false;
  }
  if (from == to)
    return true;
  //as the C# StoryDesigner: taken out and added again with the new name
  Script copy = *scr;
  copy.name = to;
  scripts.remove(from);
  scripts.add(to, copy);
  if (std::vector<PoolNode>* list = poolListOf(project_.gamePool, ResourceID::Script, from)){
    findInPool(*list, ResourceID::Script, from)->label = to;
    sortPool(*list);
  }
  scriptEditors_.erase(std::to_string(Script::Cutscene) + "/" + toLower(from));
  for (View& v : views_){
    if (v.kind == ResourceID::Script && toLower(v.name) == toLower(from))
      v.name = to;
  }
  changed();
  return true;
}

void Designer::namePopup(){
  const char* const title = "Name";
  if (nameFor_ != NameFor::None && !ImGui::IsPopupOpen(title)){
    ImGui::OpenPopup(title);
    ImGui::SetNextWindowFocus();
  }
  if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    if (!deleteName_.empty() && !ImGui::IsPopupOpen("Delete"))
      ImGui::OpenPopup("Delete");
    if (ImGui::BeginPopupModal("Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
      ImGui::Text("Delete %s?", toUtf8(deleteName_).c_str());
      const ImVec2 size(6*ImGui::GetFontSize(), 0);
      if (ImGui::Button("Delete", size)){
        deleteResource(deleteKind_, deleteName_);
        deleteName_.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel", size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
        deleteName_.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    return;
  }
  ImGui::TextUnformatted(nameFor_ == NameFor::Language ? "The new language's name:" : "The script's new name:");
  if (ImGui::IsWindowAppearing())
    ImGui::SetKeyboardFocusHere();
  bool ok = ImGui::InputText("##name", nameBuf_, sizeof(nameBuf_), ImGuiInputTextFlags_EnterReturnsTrue);
  const ImVec2 size(6*ImGui::GetFontSize(), 0);
  ok = ImGui::Button("OK", size) || ok;
  ImGui::SameLine();
  bool cancel = ImGui::Button("Cancel", size) || ImGui::IsKeyPressed(ImGuiKey_Escape);
  if (ok){
    std::string name = trim(fromUtf8(nameBuf_));
    bool done = false;
    if (nameFor_ == NameFor::RenameScript)
      done = renameScript(nameTarget_, name);
    else if (nameFor_ == NameFor::Language){
      if (name.empty())
        error_ = "A language needs a name.";
      else if (project_.languages.find(name) != nullptr)
        error_ = "There is a language called " + toUtf8(name) + " already.";
      else{
        Language lang;
        lang.name = name;
        project_.languages.add(name, lang);
        language_ = name;
        changed();
        done = true;
      }
    }
    if (done)
      cancel = true;
  }
  if (cancel){
    nameFor_ = NameFor::None;
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndPopup();
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
        if (Object* obj = project_.objects.find(v.name))
          showObject(*obj);
        else
          ImGui::TextDisabled("There is no object of this name.");
        break;
      case ResourceID::Item:
        if (Item* item = project_.items.find(v.name))
          showItem(*item);
        else
          ImGui::TextDisabled("There is no item of this name.");
        break;
      case ResourceID::Character:
        if (Character* chr = project_.characters.find(v.name))
          showCharacter(*chr);
        else
          ImGui::TextDisabled("There is no character of this name.");
        break;
      case ResourceID::Script:
        showScript(Script::Cutscene, v.name);
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
        if (v.kind == ResourceID::Image && file)
          showImage(v.name);
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
    showScript(Script::Room, room.name);
}

void Designer::showScript(Script::Type type, const std::string& name){
  Script* scr = project_.scripts[type].find(name);
  if (scr == nullptr){
    ImGui::TextDisabled("(no script)");
    if (ImGui::Button("Create script")){
      Script created;
      created.type = type;
      created.name = name;
      created.text = std::string();
      project_.scripts[type].add(name, created);
      changed();
    }
    return;
  }
  ScriptEditor& editor = scriptEditors_[std::to_string(type) + "/" + toLower(name)];
  if (editor.draw(*scr, project_))
    changed();
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
