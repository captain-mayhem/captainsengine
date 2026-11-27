#ifndef SD_DESIGNER_H
#define SD_DESIGNER_H

#include <map>
#include <string>
#include <vector>

#include <imgui.h>

#include <model/project.h>

#include "filedialog.h"
#include "imagecache.h"
#include "roomeditor.h"
#include "stateframeeditor.h"
#include "scripteditor.h"

namespace sd{

//! the project's windows (and their ImGui ids)
inline const char* const kSettings = "Project settings";
inline const char* const kCommands = "Commands and bools";
inline const char* const kLanguages = "Languages";
inline const char* const kMouseIcons = "Mouse icons";

//! the StoryDesigner's window: the project's pools and its resources in docked ImGui windows
class Designer{
public:
  Designer();
  //! opens a project file; false (and an error shown) if it can't be read
  bool openProject(const std::string& path);
  //! builds the ImGui frame
  void gui();
  //! the project's file, empty for a new project
  const std::string& path() const {return path_;}
  Project& project() {return project_;}
  //! the main menu's Quit was chosen (and the changes saved or dropped)
  bool quitRequested() const {return quit_;}
  //! the project has changes that aren't saved
  bool modified() const {return modified_;}
  //! drops the images' textures, before the render device goes
  void releaseTextures();
private:
  //! what waits for the question whether to save the changes
  enum class Pending{
    None,
    New,
    Open,
    Quit,
  };
  //! a resource shown in a window of its own
  struct View{
    ResourceID kind;
    std::string name;
    //! a script's kind, for kind Script
    Script::Type scriptType = Script::Cutscene;
    bool open = true;
    std::string title() const;
  };

  void menu();
  void layout(ImGuiID dockspace);
  void pool(const char* title, std::vector<PoolNode>& nodes, bool* open);
  void poolNodes(std::vector<PoolNode>& nodes, bool game, std::vector<size_t>& path);
  void openView(ResourceID kind, const std::string& name, Script::Type scriptType = Script::Cutscene);
  //! shows a script in a window of its own, created if there is none (as the C# editor)
  void openScript(Script::Type type, const std::string& name);
  void view(View& v);
  void settingsWindow();
  void commandsWindow();
  void languagesWindow();
  void errorPopup();
  void unsavedPopup();
  void namePopup();
  //! runs an action that drops the project, after asking to save changes
  void whenSaved(Pending action);
  void runPending();
  bool save(const std::string& path);
  void changed() {modified_ = true;}

  //! the pool's node at a path of child indices
  PoolNode* poolNode(std::vector<PoolNode>& nodes, const std::vector<size_t>& path);
  void newScript(const std::vector<size_t>& folder);
  void deleteResource(ResourceID kind, const std::string& name);
  bool renameScript(const std::string& from, const std::string& to);

  void showRoom(Room& room);
  void showObject(Object& obj);
  void showItem(Item& item);
  void showCharacter(Character& chr);
  void showImage(const std::string& name);
  void mouseIconsWindow();
  //! the script of a resource, editable; creates it on request
  void showScript(Script::Type type, const std::string& name);
  //! the state and frame editor of a resource's view
  StateFrameEditor& frameEditor(const char* kind, const std::string& name);
  //! the folder of the project's file, where its gfx.dat is
  std::string projectFolder() const;

  Project project_;
  std::string path_;
  std::vector<View> views_;
  FileDialog fileDialog_;
  bool saving_ = false;
  bool showGamePool_ = true;
  bool showMediaPool_ = true;
  bool showSettings_ = false;
  bool showCommands_ = false;
  bool showLanguages_ = false;
  bool modified_ = false;
  Pending pending_ = Pending::None;
  //! the open scripts' editors by type and lowercase name
  std::map<std::string, ScriptEditor> scriptEditors_;
  //! the state and frame editors by kind and lowercase name
  std::map<std::string, StateFrameEditor> frameEditors_;
  ImageCache images_;
  //! the room editors by lowercase name
  std::map<std::string, RoomEditor> roomEditors_;
  bool showMouseIcons_ = false;
  //! the characters' picture background (the C# editor keeps it in its settings)
  ImU32 charBackground_ = IM_COL32(0, 0, 0, 255);
  //! a character's state copied by Customize > Copy action
  CopiedState copiedState_;
  bool stateCopied_ = false;
  //! the extra state being renamed (-1: none)
  int renameState_ = -1;
  char stateNameBuf_[128] = "";
  //! the language whose translation is shown, empty for none
  std::string language_;
  //! the name popup: a new language, a new script name
  enum class NameFor{
    None,
    Language,
    RenameScript,
  };
  NameFor nameFor_ = NameFor::None;
  std::string nameTarget_;
  char nameBuf_[128] = "";
  //! a resource asked to be deleted, until confirmed
  ResourceID deleteKind_ = ResourceID::Folder;
  std::string deleteName_;
  bool layoutDone_ = false;
  ImGuiID mainDock_ = 0;
  std::string error_;
  bool quit_ = false;
};

}

#endif
