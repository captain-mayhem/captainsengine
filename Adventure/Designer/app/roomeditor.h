#ifndef SD_ROOMEDITOR_H
#define SD_ROOMEDITOR_H

#include <optional>
#include <string>

#include <imgui.h>

#include <model/project.h>
#include <model/roomedit.h>

#include "imagecache.h"

namespace sd{

//! the ImGui drag and drop payloads of an object and a character from the game pool: their names
inline const char* const kObjectPayload = "SD_OBJECT";
inline const char* const kCharacterPayload = "SD_CHARACTER";

//! the room editor (the C# RoomDlg and its control panel): the room as the
//! game shows it, with its objects and characters to place, and the
//! walkmap, depth map, inventory field and FX shapes to draw
class RoomEditor{
public:
  //! what the editor asks the rest of the StoryDesigner to do
  struct Requests{
    //! a script to show (created if there is none)
    bool openScript = false;
    Script::Type scriptType = Script::Room;
    std::string script;
    //! an object's or a character's window to show
    bool show = false;
    ResourceID showKind = ResourceID::Object;
    std::string showName;
  };
  //! draws the editor; true when the room (or the project's scripts and placed characters) changed
  bool draw(Project& project, Room& room, ImageCache& images, const std::string& folder, Requests& requests);
  RoomMode mode() const {return mode_;}
  void setMode(RoomMode mode) {mode_ = mode;}
private:
  enum class Drag{
    None,
    Entry,
    ObjectDepth,
    Walkmap,
    DepthTop,
    DepthBottom,
    InvBottom,
    InvRight,
    InvCorner,
    InvPosition,
    FxCorner,
    FxPosition,
    FxDepth,
  };
  bool canvas(Project& project, Room& room, ImageCache& images, const std::string& folder, Requests& requests);
  bool panel(Project& project, Room& room, Requests& requests);
  void drawRoom(ImDrawList* list, Project& project, Room& room, ImageCache& images, const std::string& folder);
  void drawOverlay(ImDrawList* list, const Project& project, const Room& room);
  bool mouseDown(Project& project, Room& room, ImGuiMouseButton button);
  //! the mouse cursor over the inventory field's handles
  void inventoryCursor(const Room& room) const;
  bool mouseDrag(Project& project, Room& room);
  bool drop(Project& project, Room& room);
  bool popups(Project& project, Room& room, Requests& requests);
  bool modifyWalkmap(Project& project, Room& room, Vec2i field);
  //! the selection, if it still names an object or character of the room
  bool selectionValid(const Project& project, const Room& room) const;

  RoomMode mode_ = RoomMode::Objects;
  std::optional<RoomEntry> selected_;
  Drag drag_ = Drag::None;
  Vec2i dragOffset_;
  int fxShape_ = 0;
  int fxCorner_ = 0;
  //! the mouse in the game window's pixels (the room's, less the scroll offset)
  Vec2i mouse_;
  //! the canvas: its top left on the screen and its pixels per game pixel
  ImVec2 origin_;
  float scale_ = 1;
  WalkmapPen pen_ = WalkmapPen::Size1;
  bool penFree_ = true;
  //! the walkmap field the walkmap menu was opened on
  Vec2i menuField_;
  std::optional<std::string> copiedScript_;
  bool scriptCopied_ = false;
  //! an image dropped onto the room, until background or parallax is chosen
  std::string pendingImage_;
  bool openImageMenu_ = false;
  bool openObjectMenu_ = false;
  bool openRoomMenu_ = false;
  bool openWalkmapMenu_ = false;
  bool openInventoryMenu_ = false;
  bool openRemove_ = false;
  //! the scroller's drag start: the mouse and the scroll offset
  bool scrolling_ = false;
  ImVec2 scrollStart_;
  Vec2i scrollOffsetStart_;
  //! the mirror offset pad's drag start
  int padShape_ = -1;
  ImVec2 padStart_;
  Vec2i padOffsetStart_;
};

}

#endif
