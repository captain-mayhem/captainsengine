#include "designer.h"
#include "i18n.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <functional>

#include <imgui_internal.h>

#include <platform/settings.h>
#include <platform/userdirs.h>

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

//! the window title's kind of a script that isn't a cutscene
const char* scriptKindName(Script::Type type){
  switch (type){
    case Script::Cutscene: return "Cutscene";
    case Script::Item: return "Item script";
    case Script::Character: return "Character script";
    case Script::Room: return "Room script";
    case Script::Object: return "Object script";
    case Script::Walkmap: return "Walkmap script";
    case Script::NumTypes: break;
  }
  return "Script";
}

std::string parentFolder(const std::string& path){
  std::filesystem::path p(std::u8string(path.begin(), path.end()));
  std::u8string str = p.parent_path().u8string();
  return std::string(str.begin(), str.end());
}

}

std::string Designer::View::title() const{
  //the name shown, the kind and name as the window's id
  std::string kindText = kindName(kind);
  if (kind == ResourceID::Script && scriptType != Script::Cutscene)
    kindText = scriptKindName(scriptType);
  return toUtf8(name) + " (" + kindText + ")###" + kindText + "/" + toUtf8(toLower(name));
}

void Designer::newPoolsInLanguage(){
  newPools(project_);
  //"1) CHARACTERS": the number stays
  for (PoolNode& node : project_.gamePool){
    size_t space = node.label.find(' ');
    node.label = node.label.substr(0, space+1) + fromUtf8(tr(node.label.substr(space+1).c_str()));
  }
  for (PoolNode& node : project_.mediaPool)
    node.label = fromUtf8(tr(node.label.c_str()));
}

void Designer::loadPreferences(){
  CGE::Settings& s = CGE::Settings::user();
  std::string lang = s.getString("language");
  setUiLanguage(lang == "de" ? UiLanguage::German : lang == "en" ? UiLanguage::English : systemUiLanguage());
  prefs_.createBackups = s.getBool("createBackups", prefs_.createBackups);
  prefs_.fitImages = s.getBool("fitImages", prefs_.fitImages);
  prefs_.cropTransparent = s.getBool("cropTransparentImages", prefs_.cropTransparent);
  prefs_.projectFolder = s.getString("folders/project");
  prefs_.imageFolder = s.getString("folders/images");
  prefs_.musicFolder = s.getString("folders/music");
  prefs_.soundFolder = s.getString("folders/sounds");
  prefs_.videoFolder = s.getString("folders/videos");
  charBackground_ = (ImU32)s.getInt("characterBackground", (int64_t)charBackground_);
}

void Designer::savePreferences(){
  CGE::Settings& s = CGE::Settings::user();
  //only a language chosen in the menu is kept; otherwise the system's decides
  if (s.has("language") || uiLanguage() != systemUiLanguage())
    s.setString("language", uiLanguage() == UiLanguage::German ? "de" : "en");
  s.setBool("createBackups", prefs_.createBackups);
  s.setBool("fitImages", prefs_.fitImages);
  s.setBool("cropTransparentImages", prefs_.cropTransparent);
  s.setString("folders/project", prefs_.projectFolder);
  s.setString("folders/images", prefs_.imageFolder);
  s.setString("folders/music", prefs_.musicFolder);
  s.setString("folders/sounds", prefs_.soundFolder);
  s.setString("folders/videos", prefs_.videoFolder);
  s.setInt("characterBackground", (int64_t)charBackground_);
}

std::string Designer::dialogFolder() const{
  return path_.empty() ? prefs_.projectFolder : parentFolder(path_);
}

void Designer::keepPreferences(){
  if (prefs_ == savedPrefs_ && savedLanguage_ == (int)uiLanguage() && savedCharBackground_ == charBackground_)
    return;
  savePreferences();
  savedPrefs_ = prefs_;
  savedLanguage_ = (int)uiLanguage();
  savedCharBackground_ = charBackground_;
  CGE::Settings::saveUser();
}

std::string& Designer::mediaFolder(ResourceID kind){
  switch (kind){
    case ResourceID::Music: return prefs_.musicFolder;
    case ResourceID::Sound: return prefs_.soundFolder;
    case ResourceID::Video: return prefs_.videoFolder;
    default: return prefs_.imageFolder;
  }
}

std::string Designer::backupFile() const{
  namespace fs = std::filesystem;
  static const char* const kMonths[] = {"January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"};
  std::time_t now = std::time(nullptr);
  std::tm date = *std::localtime(&now);
  //the user's data folder is in the system's narrow encoding
  fs::path dir = fs::path(CGE::UserDirs::getDataDir()) / "backups" / (std::to_string(date.tm_year + 1900) + " " + kMonths[date.tm_mon]) / std::to_string(date.tm_mday);
  std::error_code ec;
  fs::create_directories(dir, ec);
  if (ec)
    return std::string();
  //numbered after the project's backups of the day
  std::string name = toUtf8(project_.settings.projectName);
  std::string lower = toLower(name);
  int count = 1;
  for (const fs::directory_entry& entry : fs::directory_iterator(dir, ec)){
    std::u8string file = entry.path().filename().u8string();
    if (toLower(std::string(file.begin(), file.end())).compare(0, lower.size(), lower) == 0)
      ++count;
  }
  std::u8string utf8 = (dir / fs::path(std::u8string((const char8_t*)name.data(), name.size()) + u8" " + (const char8_t*)std::to_string(count).c_str() + u8".adv")).u8string();
  return std::string(utf8.begin(), utf8.end());
}

void Designer::backupTimer(){
  using namespace std::chrono;
  if (!prefs_.createBackups || !backupNeeded_ || steady_clock::now() - backupTime_ < minutes(5))
    return;
  backupTime_ = steady_clock::now();
  backupNeeded_ = false;
  std::string file = backupFile();
  //a project with a password keeps it in its backups
  Project copy = project_;
  if (!file.empty())
    writeProject(file, copy);
}

void Designer::newProject(){
  project_ = Project();
  newPoolsInLanguage();
  backupNeeded_ = false;
  backupTime_ = std::chrono::steady_clock::now();
  path_.clear();
  views_.clear();
  scriptEditors_.clear();
  frameEditors_.clear();
  roomEditors_.clear();
  images_.clear();
  stopMedia();
  language_.clear();
  gamePath_.clear();
  modified_ = false;
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
  gamePath_.clear();
  stopMedia();
  //media moved since the project was saved are looked for next to it (the C# loadFile)
  if (std::filesystem::path(std::u8string(path.begin(), path.end())).extension() == ".adv" && !resolveMedia(project_, fromUtf8(parentFolder(path))))
    error_ = tr("Not all media files were found. Project cannot work correctly.");
  views_.clear();
  scriptEditors_.clear();
  frameEditors_.clear();
  roomEditors_.clear();
  images_.clear();
  language_.clear();
  modified_ = false;
  backupNeeded_ = false;
  backupTime_ = std::chrono::steady_clock::now();
  prefs_.projectFolder = parentFolder(path);
  return true;
}

bool Designer::save(const std::string& path){
  namespace fs = std::filesystem;
  //the file as it was goes into the backups (the C# Save)
  std::error_code ec;
  fs::path file(std::u8string(path.begin(), path.end()));
  if (prefs_.createBackups && fs::is_regular_file(file, ec)){
    std::string backup = backupFile();
    if (!backup.empty())
      fs::copy_file(file, fs::path(std::u8string(backup.begin(), backup.end())), ec);
    backupTime_ = std::chrono::steady_clock::now();
    backupNeeded_ = false;
  }
  std::string error;
  if (!writeProject(path, project_, &error)){
    error_ = error;
    return false;
  }
  path_ = path;
  modified_ = false;
  prefs_.projectFolder = parentFolder(path);
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
      newProject();
      break;
    case Pending::Open:
      fileFor_ = FileFor::OpenProject;
      fileDialog_.open(FileDialog::Mode::Open, "Open a project", dialogFolder(), {".adv", ".dat"});
      break;
    case Pending::ImportAdz:
      fileFor_ = FileFor::ImportAdz;
      fileDialog_.open(FileDialog::Mode::Open, "Import an exported project", dialogFolder(), {".adz"});
      break;
    case Pending::Quit:
      quit_ = true;
      break;
    case Pending::None:
      break;
  }
}

void Designer::unsavedPopup(){
  const char* const title = tr("Unsaved changes");
  //not while the save dialog of "Save" runs
  if (pending_ != Pending::None && modified_ && !fileDialog_.isOpen() && !ImGui::IsPopupOpen(title))
    ImGui::OpenPopup(title);
  if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return;
  ImGui::Text(tr("Save the changes to %s?"), toUtf8(project_.settings.projectName).c_str());
  const ImVec2 size(7*ImGui::GetFontSize(), 0);
  if (ImGui::Button(tr("Save"), size)){
    ImGui::CloseCurrentPopup();
    if (path_.empty()){
      fileFor_ = FileFor::SaveProject;
      fileDialog_.open(FileDialog::Mode::Save, "Save the project as", dialogFolder(), {".adv"});
    }
    else if (save(path_))
      runPending();
    else
      pending_ = Pending::None;
  }
  ImGui::SameLine();
  if (ImGui::Button(tr("Don't save"), size)){
    ImGui::CloseCurrentPopup();
    modified_ = false;
    runPending();
  }
  ImGui::SameLine();
  if (ImGui::Button(tr("Cancel"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
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
  if (showFonts_)
    fontsWindow();
  const bool dialogWasOpen = fileDialog_.isOpen();
  if (fileDialog_.draw())
    fileChosen();
  else if (dialogWasOpen && !fileDialog_.isOpen() && fileFor_ == FileFor::SaveProject)
    pending_ = Pending::None; //the save was cancelled, and what waited for it
  unsavedPopup();
  namePopup();
  deletePopup();
  taskPopup();
  errorPopup();
  if (showAbout_)
    aboutWindow();
  if (showRemote_)
    remoteWindow();
  backupTimer();
  keepPreferences();
}

void Designer::insertMenu(){
  if (!ImGui::BeginMenu(tr("Insert")))
    return;
  //into the kind's top folder, as the C# Insert menu does
  static const std::pair<const char*, ResourceID> kMedia[] = {{"Image...", ResourceID::Image}, {"Soundeffect...", ResourceID::Sound},
    {"Musicfile...", ResourceID::Music}, {"Video...", ResourceID::Video}};
  for (const auto& media : kMedia){
    if (ImGui::MenuItem(tr(media.first)))
      addMediaFiles(media.second, PoolPath{(size_t)topFolder(media.second)});
  }
  ImGui::Separator();
  static const std::pair<const char*, ResourceID> kResources[] = {{"Room", ResourceID::Room}, {"Object", ResourceID::Object},
    {"Character", ResourceID::Character}, {"Item", ResourceID::Item}, {"Script", ResourceID::Script}};
  for (const auto& res : kResources){
    if (ImGui::MenuItem(tr(res.first)))
      newResource(res.second, PoolPath{(size_t)topFolder(res.second)});
  }
  ImGui::EndMenu();
}

void Designer::menu(){
  if (!ImGui::BeginMainMenuBar())
    return;
  if (ImGui::BeginMenu(tr("File"))){
    if (ImGui::MenuItem(tr("New")))
      whenSaved(Pending::New);
    if (ImGui::MenuItem(tr("Open...")))
      whenSaved(Pending::Open);
    if (ImGui::MenuItem(tr("Save"), nullptr, false, !path_.empty()))
      save(path_);
    if (ImGui::MenuItem(tr("Save as..."))){
      fileFor_ = FileFor::SaveProject;
      fileDialog_.open(FileDialog::Mode::Save, "Save the project as", dialogFolder(), {".adv"});
    }
    ImGui::Separator();
    //the imported project replaces this one
    if (ImGui::MenuItem(tr("Import project (.adz)...")))
      whenSaved(Pending::ImportAdz);
    if (ImGui::MenuItem(tr("Export project (.adz)..."))){
      fileFor_ = FileFor::ExportAdz;
      fileDialog_.open(FileDialog::Mode::Save, "Export the project", dialogFolder(), {".adz"});
    }
    if (ImGui::MenuItem(tr("Import character (.adc)..."))){
      fileFor_ = FileFor::ImportCharacter;
      fileDialog_.open(FileDialog::Mode::Open, "Import a character", dialogFolder(), {".adc"});
    }
    if (ImGui::MenuItem(tr("Import room (.adr)..."))){
      fileFor_ = FileFor::ImportRoom;
      fileDialog_.open(FileDialog::Mode::Open, "Import a room", dialogFolder(), {".adr"});
    }
    ImGui::Separator();
    if (ImGui::MenuItem(tr("Create game..."), nullptr, false, !task_)){
      fileFor_ = FileFor::CreateGame;
      //the game made before is offered again (the C# "update the previous game")
      std::string folder = gamePath_.empty() ? toUtf8(project_.settings.directory) : parentFolder(gamePath_);
      //a folder of another computer gives the project's
      std::error_code ec;
      if (folder.empty() || !std::filesystem::is_directory(std::filesystem::path(std::u8string(folder.begin(), folder.end())), ec))
        folder = dialogFolder();
      fileDialog_.open(FileDialog::Mode::Save, "Create the game as", folder, {});
      if (gamePath_.empty())
        fileDialog_.setName(toUtf8(project_.settings.projectName));
      else{
        std::u8string file = std::filesystem::path(std::u8string(gamePath_.begin(), gamePath_.end())).filename().u8string();
        fileDialog_.setName(std::string(file.begin(), file.end()));
      }
    }
    if (ImGui::MenuItem(tr("Run game"), nullptr, false, !task_))
      startRunGame();
    ImGui::Separator();
    if (ImGui::MenuItem(tr("Quit")))
      whenSaved(Pending::Quit);
    ImGui::EndMenu();
  }
  insertMenu();
  if (ImGui::BeginMenu(tr("Project"))){
    ImGui::MenuItem(tr("Settings"), nullptr, &showSettings_);
    ImGui::MenuItem(tr("Commands and bools"), nullptr, &showCommands_);
    ImGui::MenuItem(tr(kMouseIcons), nullptr, &showMouseIcons_);
    ImGui::MenuItem(tr(kFonts), nullptr, &showFonts_);
    if (ImGui::MenuItem(tr("Languages"), nullptr, &showLanguages_) && showLanguages_){
      //the C# StoryDesigner gathers the texts when its language tools open
      updateLanguageList(project_);
      changed();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(tr("View"))){
    ImGui::MenuItem(tr(kGamePool), nullptr, &showGamePool_);
    ImGui::MenuItem(tr(kMediaPool), nullptr, &showMediaPool_);
    //the room editors' views (the C# "Walkmap on" and so on): on, or off back to the objects
    ImGui::Separator();
    static const std::pair<const char*, RoomMode> kModes[] = {{"Walkmap on", RoomMode::Walkmap}, {"Deepmap on", RoomMode::Deepmap},
      {"Inventoryfield on", RoomMode::Inventory}, {"FX Shapes on", RoomMode::SpecialFx}};
    for (const auto& mode : kModes){
      if (ImGui::MenuItem(tr(mode.first), nullptr, roomMode_ == mode.second))
        roomMode_ = roomMode_ == mode.second ? RoomMode::Objects : mode.second;
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(tr("Plugins"))){
    ImGui::MenuItem(tr("Remote control"), nullptr, &showRemote_);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(tr("Language"))){
    //the C# StoryDesigner needs a restart for this; here it changes at once
    if (ImGui::MenuItem("English", nullptr, uiLanguage() == UiLanguage::English))
      setUiLanguage(UiLanguage::English);
    if (ImGui::MenuItem("Deutsch", nullptr, uiLanguage() == UiLanguage::German))
      setUiLanguage(UiLanguage::German);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(tr("Help"))){
    ImGui::MenuItem(tr("About..."), nullptr, &showAbout_);
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
  ImGui::DockBuilderDockWindow(windowTitle(kGamePool), top);
  ImGui::DockBuilderDockWindow(windowTitle(kMediaPool), bottom);
  ImGui::DockBuilderDockWindow(windowTitle(kSettings), right);
  ImGui::DockBuilderDockWindow(windowTitle(kCommands), right);
  ImGui::DockBuilderDockWindow(windowTitle(kLanguages), right);
  ImGui::DockBuilderDockWindow(windowTitle(kMouseIcons), right);
  ImGui::DockBuilderDockWindow(windowTitle(kFonts), right);
  ImGui::DockBuilderFinish(dockspace);
  mainDock_ = right;
}

void Designer::pool(const char* title, std::vector<PoolNode>& nodes, bool* open){
  if (ImGui::Begin(windowTitle(title), open)){
    if (nodes.empty())
      ImGui::TextDisabled("(empty)");
    std::vector<size_t> path;
    poolNodes(nodes, &nodes == &project_.gamePool, path);
  }
  ImGui::End();
  std::vector<std::function<void()>> actions;
  actions.swap(poolActions_);
  for (auto& action : actions)
    action();
}

void Designer::poolNodes(std::vector<PoolNode>& nodes, bool game, std::vector<size_t>& path){
  //what the menus and drops do to the pool waits until it is drawn (poolActions_)
  for (size_t i = 0; i < nodes.size(); ++i){
    PoolNode& node = nodes[i];
    std::string label = toUtf8(node.label);
    ImGui::PushID((int)i);
    path.push_back(i);
    if (node.id == (int)ResourceID::Folder){
      bool open = ImGui::TreeNodeEx("##folder", ImGuiTreeNodeFlags_SpanAvailWidth, "%s/", label.c_str());
      poolDragDrop(node, game, path);
      poolMenu(node, game, path);
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
      poolDragDrop(node, game, path);
      poolMenu(node, game, path);
    }
    path.pop_back();
    ImGui::PopID();
  }
}

namespace{

//! a pool path as a drag and drop payload: the pool, then the indices
std::string encodePath(bool game, const PoolPath& path){
  std::string out = game ? "g" : "m";
  for (size_t idx : path)
    out += "/" + std::to_string(idx);
  return out;
}

bool decodePath(const std::string& text, bool& game, PoolPath& path){
  if (text.empty() || (text[0] != 'g' && text[0] != 'm'))
    return false;
  game = text[0] == 'g';
  path.clear();
  size_t pos = 1;
  while (pos < text.size() && text[pos] == '/'){
    size_t end = text.find('/', pos+1);
    path.push_back((size_t)std::stoul(text.substr(pos+1, end == std::string::npos ? std::string::npos : end-pos-1)));
    pos = end == std::string::npos ? text.size() : end;
  }
  return true;
}

const char* const kPoolPayload = "SD_POOLNODE";

const char* newName(ResourceID kind){
  switch (kind){
    case ResourceID::Character: return "New character";
    case ResourceID::Script: return "New script";
    case ResourceID::Item: return "New item";
    case ResourceID::Object: return "New object";
    case ResourceID::Room: return "New room";
    case ResourceID::Image: return "Add images...";
    case ResourceID::Music: return "Add music...";
    case ResourceID::Sound: return "Add sounds...";
    case ResourceID::Video: return "Add videos...";
    case ResourceID::Folder: break;
  }
  return "New";
}

}

void Designer::poolMenu(PoolNode& node, bool game, const PoolPath& path){
  if (!ImGui::BeginPopupContextItem())
    return;
  std::vector<PoolNode>& pool = game ? project_.gamePool : project_.mediaPool;
  ResourceID kind = kindAt(pool, game, path);
  bool folder = node.id == (int)ResourceID::Folder;
  if (folder){
    if (kind != ResourceID::Folder && ImGui::MenuItem(newName(kind))){
      if (game)
        poolActions_.push_back([this, kind, path]{newResource(kind, path);});
      else
        addMediaFiles(kind, path);
    }
    if (ImGui::MenuItem(tr("New folder"))){
      poolActions_.push_back([this, game, path]{
        std::vector<PoolNode>& pool = game ? project_.gamePool : project_.mediaPool;
        PoolPath created = newFolder(project_, game, path);
        if (created.empty())
          return;
        changed();
        //named right away, as the C# tree view edits its label
        nameFor_ = NameFor::RenameFolder;
        nameGame_ = game;
        namePath_ = created;
        snprintf(nameBuf_, sizeof(nameBuf_), "%s", toUtf8(poolNode(pool, created)->label).c_str());
      });
    }
    //the top folders stay as they are
    if (path.size() > 1){
      ImGui::Separator();
      if (ImGui::MenuItem(tr("Rename..."))){
        nameFor_ = NameFor::RenameFolder;
        nameGame_ = game;
        namePath_ = path;
        snprintf(nameBuf_, sizeof(nameBuf_), "%s", toUtf8(node.label).c_str());
      }
      if (ImGui::MenuItem(tr("Delete")))
        askDelete(game, path);
    }
  }
  else{
    ResourceID id = (ResourceID)node.id;
    if (ImGui::MenuItem(tr("Open")))
      openView(id, node.label);
    //the media keep their files' names
    if (game){
      if (ImGui::MenuItem(tr("Rename..."))){
        nameFor_ = NameFor::Rename;
        nameKind_ = id;
        nameTarget_ = node.label;
        snprintf(nameBuf_, sizeof(nameBuf_), "%s", toUtf8(node.label).c_str());
      }
      if (ImGui::MenuItem(tr("Duplicate"))){
        std::string label = node.label;
        poolActions_.push_back([this, id, label]{duplicateResource(id, label);});
      }
    }
    if (ImGui::MenuItem(tr("Delete")))
      askDelete(game, path);
  }
  ImGui::EndPopup();
}

void Designer::poolDragDrop(PoolNode& node, bool game, const PoolPath& path){
  //images, objects and characters are dragged into frames and rooms by
  //name, the rest by their place in the pool
  const char* payload = nullptr;
  if (node.id == (int)ResourceID::Image)
    payload = kImagePayload;
  else if (node.id == (int)ResourceID::Object)
    payload = kObjectPayload;
  else if (node.id == (int)ResourceID::Character)
    payload = kCharacterPayload;
  if (path.size() > 1 && ImGui::BeginDragDropSource()){
    if (payload)
      ImGui::SetDragDropPayload(payload, node.label.data(), node.label.size());
    else{
      std::string encoded = encodePath(game, path);
      ImGui::SetDragDropPayload(kPoolPayload, encoded.data(), encoded.size());
    }
    ImGui::TextUnformatted(toUtf8(node.label).c_str());
    ImGui::EndDragDropSource();
  }
  if (node.id != (int)ResourceID::Folder || !ImGui::BeginDragDropTarget())
    return;
  std::vector<PoolNode>& pool = game ? project_.gamePool : project_.mediaPool;
  ResourceID kind = kindAt(pool, game, path);
  bool fromGame = game;
  PoolPath from;
  if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(kPoolPayload)){
    if (!decodePath(std::string((const char*)p->Data, p->DataSize), fromGame, from))
      from.clear();
  }
  else{
    struct Named{
      const char* payload;
      ResourceID kind;
      bool game;
    };
    for (const Named& n : {Named{kImagePayload, ResourceID::Image, false}, Named{kObjectPayload, ResourceID::Object, true}, Named{kCharacterPayload, ResourceID::Character, true}}){
      if (n.game != game || n.kind != kind)
        continue;
      if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(n.payload)){
        from = pathInPool(pool, n.kind, std::string((const char*)p->Data, p->DataSize));
        fromGame = n.game;
      }
    }
  }
  if (!from.empty() && fromGame == game){
    poolActions_.push_back([this, game, from, path]{
      if (movePoolNode(project_, game, from, path))
        changed();
    });
  }
  ImGui::EndDragDropTarget();
}

void Designer::newResource(ResourceID kind, const PoolPath& folder){
  std::string name = sd::newResource(project_, kind, folder);
  if (name.empty())
    return;
  changed();
  openView(kind, name);
  //named right away, as the C# StoryDesigner edits the new node's label
  nameFor_ = NameFor::Rename;
  nameKind_ = kind;
  nameTarget_ = name;
  snprintf(nameBuf_, sizeof(nameBuf_), "%s", toUtf8(name).c_str());
}

void Designer::duplicateResource(ResourceID kind, const std::string& name){
  std::string copy = sd::duplicateResource(project_, kind, name);
  if (copy.empty())
    return;
  changed();
  openView(kind, copy);
  nameFor_ = NameFor::Rename;
  nameKind_ = kind;
  nameTarget_ = copy;
  snprintf(nameBuf_, sizeof(nameBuf_), "%s", toUtf8(copy).c_str());
}

void Designer::askDelete(bool game, const PoolPath& path){
  std::vector<PoolNode>& pool = game ? project_.gamePool : project_.mediaPool;
  PoolNode* node = poolNode(pool, path);
  if (node == nullptr || path.size() < 2)
    return;
  if (project_.settings.silentDelete){
    poolActions_.push_back([this, game, path]{deleteNode(game, path);});
    return;
  }
  deleteAsked_ = true;
  deleteGame_ = game;
  deletePath_ = path;
  if (node->id == (int)ResourceID::Folder)
    deleteText_ = "Delete subdirectory " + toUtf8(node->label) + " and all its entries?";
  else
    deleteText_ = "Delete " + toUtf8(node->label) + "?";
}

void Designer::deleteNode(bool game, const PoolPath& path){
  deletePoolNode(project_, game, path);
  pruneViews();
  changed();
}

void Designer::deletePopup(){
  if (deleteAsked_ && !ImGui::IsPopupOpen(tr("Delete")))
    ImGui::OpenPopup(tr("Delete"));
  if (!ImGui::BeginPopupModal(tr("Delete"), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return;
  ImGui::TextUnformatted(deleteText_.c_str());
  const ImVec2 size(6*ImGui::GetFontSize(), 0);
  if (ImGui::Button(tr("Delete"), size)){
    deleteNode(deleteGame_, deletePath_);
    deleteAsked_ = false;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(tr("Cancel"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
    deleteAsked_ = false;
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndPopup();
}

bool Designer::exists(ResourceID kind, const std::string& name, Script::Type scriptType) const{
  switch (kind){
    case ResourceID::Room: return project_.rooms.find(name) != nullptr;
    case ResourceID::Object: return project_.objects.find(name) != nullptr;
    case ResourceID::Item: return project_.items.find(name) != nullptr;
    case ResourceID::Character: return project_.characters.find(name) != nullptr;
    case ResourceID::Script: return project_.scripts[scriptType].find(name) != nullptr;
    case ResourceID::Folder: return false;
    default:{
      const NamedList<std::string>* files = mediaFiles(project_, kind);
      return files && files->find(name) != nullptr;
    }
  }
}

void Designer::pruneViews(){
  for (View& v : views_){
    if (!exists(v.kind, v.name, v.scriptType))
      v.open = false;
  }
  //the editors of what is gone; the others keep their state
  for (auto it = roomEditors_.begin(); it != roomEditors_.end();){
    if (!project_.rooms.find(it->first))
      it = roomEditors_.erase(it);
    else
      ++it;
  }
  for (auto it = scriptEditors_.begin(); it != scriptEditors_.end();){
    size_t slash = it->first.find('/');
    int type = std::atoi(it->first.substr(0, slash).c_str());
    if (type < 0 || type >= Script::NumTypes || !project_.scripts[type].find(it->first.substr(slash+1)))
      it = scriptEditors_.erase(it);
    else
      ++it;
  }
}

bool Designer::rename(ResourceID kind, const std::string& from, const std::string& to){
  std::string error;
  if (!renameResource(project_, kind, from, to, &error)){
    error_ = error;
    return false;
  }
  if (from == to)
    return true;
  for (View& v : views_){
    if (v.kind == kind && toLower(v.name) == toLower(from))
      v.name = to;
  }
  //the editors keyed by the old name start again under the new one
  frameEditors_.clear();
  roomEditors_.erase(toLower(from));
  scriptEditors_.clear();
  pruneViews();
  changed();
  return true;
}

void Designer::addMediaFiles(ResourceID kind, const PoolPath& folder){
  static const std::vector<std::string> images{".jpg", ".jpeg", ".png", ".bmp", ".tga", ".gif"};
  static const std::vector<std::string> sounds{".wav", ".ogg", ".mp3", ".mp2", ".mp1"};
  static const std::vector<std::string> music{".wav", ".ogg", ".mp3", ".mp2", ".mp1", ".it", ".xm", ".mod", ".s3m", ".mo3", ".umx"};
  static const std::vector<std::string> videos{".avi", ".swf", ".mpeg", ".mpg", ".wmv", ".ogv"};
  const std::vector<std::string>* exts = &images;
  const char* title = "Add images";
  if (kind == ResourceID::Sound){
    exts = &sounds;
    title = "Add sound effects";
  }
  else if (kind == ResourceID::Music){
    exts = &music;
    title = "Add music";
  }
  else if (kind == ResourceID::Video){
    exts = &videos;
    title = "Add videos";
  }
  fileFor_ = FileFor::AddMedia;
  mediaKind_ = kind;
  mediaFolder_ = folder;
  fileDialog_.open(FileDialog::Mode::Open, title, mediaFolder(kind).empty() ? dialogFolder() : mediaFolder(kind), *exts, true);
}

void Designer::namePopup(){
  const char* const title = "Name";
  if (nameFor_ != NameFor::None && !ImGui::IsPopupOpen(title)){
    ImGui::OpenPopup(title);
    ImGui::SetNextWindowFocus();
  }
  if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return;
  if (nameFor_ == NameFor::Language)
    ImGui::TextUnformatted(tr("The new language's name:"));
  else if (nameFor_ == NameFor::RenameFolder)
    ImGui::TextUnformatted(tr("The folder's name:"));
  else
    ImGui::Text("The %s's name:", kindName(nameKind_));
  if (ImGui::IsWindowAppearing())
    ImGui::SetKeyboardFocusHere();
  bool ok = ImGui::InputText("##name", nameBuf_, sizeof(nameBuf_), ImGuiInputTextFlags_EnterReturnsTrue);
  const ImVec2 size(6*ImGui::GetFontSize(), 0);
  ok = ImGui::Button(tr("OK"), size) || ok;
  ImGui::SameLine();
  bool cancel = ImGui::Button(tr("Cancel"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape);
  if (ok){
    std::string name = trim(fromUtf8(nameBuf_));
    bool done = false;
    if (nameFor_ == NameFor::Rename)
      done = rename(nameKind_, nameTarget_, name);
    else if (nameFor_ == NameFor::RenameFolder){
      PoolNode* node = poolNode(nameGame_ ? project_.gamePool : project_.mediaPool, namePath_);
      if (node && node->label == name)
        done = true;
      else if (renameFolder(project_, nameGame_, namePath_, name)){
        changed();
        done = true;
      }
      else
        error_ = name.empty() ? "A folder needs a name." : "There is a folder called " + toUtf8(name) + " already.";
    }
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

void Designer::openView(ResourceID kind, const std::string& name, Script::Type scriptType){
  for (View& v : views_){
    if (v.kind == kind && v.scriptType == scriptType && toLower(v.name) == toLower(name)){
      ImGui::SetWindowFocus(v.title().c_str());
      return;
    }
  }
  View v;
  v.kind = kind;
  v.name = name;
  v.scriptType = scriptType;
  views_.push_back(v);
}

void Designer::openScript(Script::Type type, const std::string& name){
  if (project_.scripts[type].find(name) == nullptr){
    Script created;
    created.type = type;
    created.name = name;
    created.text = std::string();
    project_.scripts[type].add(name, created);
    changed();
  }
  openView(ResourceID::Script, name, type);
}

void Designer::view(View& v){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(40*ImGui::GetFontSize(), 30*ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
  if (ImGui::Begin(v.title().c_str(), &v.open)){
    switch (v.kind){
      case ResourceID::Room:
        if (Room* room = project_.rooms.find(v.name))
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
        showScript(v.scriptType, v.name);
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
        else if (file)
          showMedia(v.kind, v.name);
        break;
      }
    }
  }
  ImGui::End();
}

void Designer::showRoom(Room& room){
  RoomEditor& editor = roomEditors_[toLower(room.name)];
  RoomEditor::Requests requests;
  //one view for all rooms; a key in one room changes it for all
  editor.setMode(roomMode_);
  if (editor.draw(project_, room, images_, projectFolder(), requests))
    changed();
  roomMode_ = editor.mode();
  if (requests.openScript)
    openScript(requests.scriptType, requests.script);
  if (requests.show)
    openView(requests.showKind, requests.showName);
}

void Designer::showScript(Script::Type type, const std::string& name){
  Script* scr = project_.scripts[type].find(name);
  if (scr == nullptr){
    ImGui::TextDisabled("(no script)");
    if (ImGui::Button(tr("Create script"))){
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
  if (!error_.empty() && !ImGui::IsPopupOpen(tr("Error")))
    ImGui::OpenPopup(tr("Error"));
  if (ImGui::BeginPopupModal(tr("Error"), nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    ImGui::TextUnformatted(error_.c_str());
    if (ImGui::Button(tr("OK"), ImVec2(6*ImGui::GetFontSize(), 0))){
      error_.clear();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

}
