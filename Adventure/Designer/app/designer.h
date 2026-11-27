#ifndef SD_DESIGNER_H
#define SD_DESIGNER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <imgui.h>

#include <model/project.h>
#include <model/resources.h>

#include "filedialog.h"
#include "imagecache.h"
#include "remotecontrol.h"
#include "roomeditor.h"
#include "stateframeeditor.h"
#include "scripteditor.h"

namespace sd{

//! the project's windows (and their ImGui ids)
inline const char* const kSettings = "Project settings";
inline const char* const kCommands = "Commands and bools";
inline const char* const kLanguages = "Languages";
inline const char* const kMouseIcons = "Mouse icons";
inline const char* const kFonts = "Fonts";

class MediaPreview;
class FontPreview;

//! the StoryDesigner's window: the project's pools and its resources in docked ImGui windows
class Designer{
public:
  Designer();
  ~Designer();
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
    ImportAdz,
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
  void deletePopup();
  //! runs an action that drops the project, after asking to save changes
  void whenSaved(Pending action);
  void runPending();
  bool save(const std::string& path);
  void changed() {modified_ = true; backupNeeded_ = true;}

  //! the context menu of a pool's node
  void poolMenu(PoolNode& node, bool game, const PoolPath& path);
  //! a pool's node as a drag source and as a drop target for moving nodes into folders
  void poolDragDrop(PoolNode& node, bool game, const PoolPath& path);
  //! makes a new resource in a folder and shows it
  void newResource(ResourceID kind, const PoolPath& folder);
  void duplicateResource(ResourceID kind, const std::string& name);
  //! asks to delete a pool's node (unless the project deletes silently)
  void askDelete(bool game, const PoolPath& path);
  void deleteNode(bool game, const PoolPath& path);
  bool rename(ResourceID kind, const std::string& from, const std::string& to);
  //! closes the views of resources that are gone, and drops their editors
  void pruneViews();
  bool exists(ResourceID kind, const std::string& name, Script::Type scriptType) const;
  //! the file dialog for adding media files of a kind into a folder
  void addMediaFiles(ResourceID kind, const PoolPath& folder);
  //! a file the file dialog chose, for what it was opened for
  void fileChosen();
  //! a new project with the pools' folders
  void newProject();
  //! the new project's pools, their top folders named in the StoryDesigner's language
  void newPoolsInLanguage();
  //! the Insert menu: a new resource or media files in its top folder
  void insertMenu();
  void aboutWindow();
  //! the C# RemoteControl plugin: the running game driven over the network
  void remoteWindow();
  //! the editor's own settings (the C# Persistence), kept in the user's settings
  void loadPreferences();
  void savePreferences();
  //! a backup's file, in the user's data folder by date (the C# getBackupFileName); empty if it can't be made
  std::string backupFile() const;
  //! writes a backup every five minutes while there are changes
  void backupTimer();

  void showRoom(Room& room);
  void showObject(Object& obj);
  void showItem(Item& item);
  void showCharacter(Character& chr);
  void showImage(const std::string& name);
  //! a sound's, music's or video's player
  void showMedia(ResourceID kind, const std::string& name);
  //! stops and drops the media player
  void stopMedia();
  void fontsWindow();
  //! Create game into the game's path; Run game, in a folder of its own
  void startCreateGame(const std::string& path);
  void startRunGame();
  //! the running task's window, and what comes when it ended
  void taskPopup();
  //! where Create game finds the runtime (engine1.dat, engine2.dat, font.dat)
  std::string runtimeFolder() const;
  //! starts the game made by Create game or Run game
  void startGame(const std::string& program, const std::vector<std::string>& args, const std::string& folder);
  void importFile(const std::string& path);
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
  //! what the file dialog is open for
  enum class FileFor{
    OpenProject,
    SaveProject,
    AddMedia,
    ImportAdz,
    ExportAdz,
    ImportCharacter,
    ImportRoom,
    CreateGame,
  };
  FileFor fileFor_ = FileFor::OpenProject;
  //! the media kind and pool folder the files are added for
  ResourceID mediaKind_ = ResourceID::Image;
  PoolPath mediaFolder_;
  //! the folder the media files were added from last (UTF-8)
  //! the editor's own settings (the C# Persistence)
  struct Preferences{
    bool createBackups = true;
    //! an object's first image sets its size (the C# "Fit images")
    bool fitImages = true;
    //! added images lose their transparent border (changing the files)
    bool cropTransparent = false;
    //! the folders the projects and each kind of media were taken from last (UTF-8)
    std::string projectFolder;
    std::string imageFolder;
    std::string musicFolder;
    std::string soundFolder;
    std::string videoFolder;
    bool operator==(const Preferences&) const = default;
  };
  Preferences prefs_;
  //! the preferences as they were saved last, and the language and background with them
  Preferences savedPrefs_;
  int savedLanguage_ = -1;
  ImU32 savedCharBackground_ = 0;
  //! saves the preferences when they changed
  void keepPreferences();
  //! where the file dialogs for projects start: the project's folder, or where the last one was
  std::string dialogFolder() const;
  //! the folder media of a kind were added from last
  std::string& mediaFolder(ResourceID kind);
  //! changes since the last backup, and when it was written (or the project saved or opened)
  bool backupNeeded_ = false;
  std::chrono::steady_clock::time_point backupTime_ = std::chrono::steady_clock::now();
  //! the room editors' view, shared by all of them as in the C# editor
  RoomMode roomMode_ = RoomMode::Objects;
  bool showAbout_ = false;
  bool showRemote_ = false;
  std::unique_ptr<RemoteControl> remote_;
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
  bool showFonts_ = false;
  int font_ = 0;
  std::vector<std::string> fontFamilies_;
  std::unique_ptr<FontPreview> fontPreview_;
  std::unique_ptr<MediaPreview> media_;
  //! the game made last, offered again by Create game
  std::string gamePath_;
  //! Create game and Run game run in a thread; the window shows the step
  struct Task{
    bool run = false;
    std::thread thread;
    std::atomic<bool> done{false};
    std::mutex mutex;
    std::string step;
    bool ok = false;
    std::string error;
    std::vector<std::string> warnings;
    //! the project as it was given to the thread, for the password it may get
    Project project;
    //! the game's program, its arguments and folder
    std::string program;
    std::vector<std::string> args;
    std::string folder;
  };
  std::unique_ptr<Task> task_;
  //! what the pools' menus and drops do, after the pool is drawn
  std::vector<std::function<void()>> poolActions_;
  //! the question whether to start the game that was made
  bool askStart_ = false;
  std::string message_;
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
  //! the name popup: a new language, a resource's or folder's new name
  enum class NameFor{
    None,
    Language,
    Rename,
    RenameFolder,
  };
  NameFor nameFor_ = NameFor::None;
  ResourceID nameKind_ = ResourceID::Script;
  std::string nameTarget_;
  bool nameGame_ = true;
  PoolPath namePath_;
  char nameBuf_[128] = "";
  //! a pool's node asked to be deleted, until confirmed
  bool deleteAsked_ = false;
  bool deleteGame_ = true;
  PoolPath deletePath_;
  std::string deleteText_;
  bool layoutDone_ = false;
  ImGuiID mainDock_ = 0;
  std::string error_;
  bool quit_ = false;
};

}

#endif
