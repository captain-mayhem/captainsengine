#ifndef SD_DESIGNER_H
#define SD_DESIGNER_H

#include <string>
#include <vector>

#include <imgui.h>

#include <model/project.h>

#include "filedialog.h"

namespace sd{

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
  //! the main menu's Quit was chosen
  bool quitRequested() const {return quit_;}
private:
  //! a resource shown in a window of its own
  struct View{
    ResourceID kind;
    std::string name;
    bool open = true;
    std::string title() const;
  };

  void menu();
  void layout(ImGuiID dockspace);
  void pool(const char* title, std::vector<PoolNode>& nodes, bool* open);
  void poolNodes(std::vector<PoolNode>& nodes);
  void openView(ResourceID kind, const std::string& name);
  void view(View& v);
  void settingsWindow();
  void errorPopup();
  bool save(const std::string& path);

  void showRoom(const Room& room);
  void showObject(const Object& obj);
  void showItem(const Item& item);
  void showCharacter(const Character& chr);
  void showScript(const Script* scr);
  void showFrames(const std::vector<ExtendedFrame>& frames);

  Project project_;
  std::string path_;
  std::vector<View> views_;
  FileDialog fileDialog_;
  bool saving_ = false;
  bool showGamePool_ = true;
  bool showMediaPool_ = true;
  bool showSettings_ = false;
  bool layoutDone_ = false;
  ImGuiID mainDock_ = 0;
  std::string error_;
  bool quit_ = false;
};

}

#endif
